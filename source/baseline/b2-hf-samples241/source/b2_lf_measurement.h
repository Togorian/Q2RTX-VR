// Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
/* Modified 11 November 2026: added provenance notice for the Q2RTX VR
 * source selection. Earlier adaptations retain their original notices;
 * see docs/MODIFICATIONS.md and docs/PROVENANCE.md. */
// SPDX-License-Identifier: GPL-2.0-or-later
// Diagnostic241: bounded command-armed successor of the R64 HF edge observation. Existing B2 serial completion/readback and
// FS writer APIs; same device, descriptor, readback topology, waits and threads; extended diagnostic extent.
#ifdef REFERENCE_B_B2
#include "b2_hf_samples.h"
#define B2M_MAGIC UINT32_C(0xB2604C46)
#define B2M_RECORD_WORDS 1504u
#define B2M_META_WORDS 229u
#define B2M_MAX_RECORDS 128u
#define B2M_MAX_FRAMES 64u
#define B2M_BYTE_CAP 770304u
static struct {
    unsigned label;
    qhandle_t file;
    bool attempted, stopped, tail_owned, tail_dirty, armed;
    uint32_t frames, records, bytes, first_ms, previous_write_ms, max_snapshot_ms, max_write_ms;
    uint32_t metadata[B2M_META_WORDS], control[24], compact[60];
} b2m;
static uint32_t b2m_bits(float f){uint32_t u;memcpy(&u,&f,4);return u;}

// R51 fixed structural entries come from actual B2_ViewFromPose. Store every
// variable coefficient without quantisation; unsupported structure is flagged.
static bool b2m_pack_matrix(uint32_t *out,const mat4 matrix,unsigned kind)
{
    float m[16];memcpy(m,matrix,sizeof(m));
    static const unsigned indices[3][12]={
        {0,1,2,4,5,6,8,9,10,12,13,14},
        {0,5,8,9,10,14}, {0,5,11,12,13,15}};
    unsigned count=kind==0?12:6,fixed=kind==0?15:kind==1?11:14;
    bool supported=true;
    for(unsigned i=0;i<16;i++){
        bool variable=false;for(unsigned j=0;j<count;j++)if(indices[kind][j]==i)variable=true;
        if(!isfinite(m[i])||(!variable&&m[i]!=(i==fixed?1.f:0.f)))supported=false;
    }
    for(unsigned j=0;j<count;j++)out[j]=b2m_bits(m[indices[kind][j]]);
    return supported;
}
static bool b2m_pack_matrices(const QVKUniformBuffer_t *u,uint32_t out[60])
{
    bool ok=true;
    ok=b2m_pack_matrix(out,u->V,0)&&ok;
    ok=b2m_pack_matrix(out+12,u->P,1)&&ok;
    ok=b2m_pack_matrix(out+18,u->invV,0)&&ok;
    ok=b2m_pack_matrix(out+30,u->invP,2)&&ok;
    ok=b2m_pack_matrix(out+36,u->V_prev,0)&&ok;
    ok=b2m_pack_matrix(out+48,u->P_prev,1)&&ok;
    ok=b2m_pack_matrix(out+54,u->invP_prev,2)&&ok;
    return ok;
}

static bool b2m_write(const void *p,size_t n)
{
    if(!b2m.file||b2m.bytes>B2M_BYTE_CAP||n>B2M_BYTE_CAP-b2m.bytes)return false;
    int written=FS_Write(p,n,b2m.file);
    if(written>0)b2m.bytes+=(uint32_t)written;
    return written==(int)n;
}
static void b2m_close(uint32_t reason)
{
    if(b2m.stopped)return;
    if(b2m.file){
        uint32_t footer[32]={UINT32_C(0x52363445),1,128,0};
        footer[4]=reason;footer[5]=b2m.frames;footer[6]=b2m.records;footer[7]=b2m.bytes;
        footer[8]=b2m.max_snapshot_ms;footer[9]=b2m.max_write_ms;
        footer[10]=b2m.label;footer[11]=241;
        bool complete=b2m_write(footer,sizeof(footer));
        int closed=FS_CloseFile(b2m.file);b2m.file=0;
        Com_Printf("HF241 END label=%u complete=%d reason=%u frames=%u records=%u bytes=%u footer=%d close=%d snapshot_max_ms=%u write_max_ms=%u\n",
            b2m.label,reason==1 && b2m.frames==64 && b2m.records==128 && complete && closed>=0,
            reason,b2m.frames,b2m.records,b2m.bytes,complete,closed,b2m.max_snapshot_ms,b2m.max_write_ms);
    }
    b2m.stopped=true;b2m.armed=false;
}
static bool b2m_open(void)
{
    if(b2m.attempted)return b2m.file!=0;
    b2m.attempted=true;b2m.first_ms=Sys_Milliseconds();
    const char *name=b2m.label==1 ? "hf241-on.bin" : "hf241-off.bin";
    if(FS_OpenFile(name,&b2m.file,FS_MODE_WRITE|FS_FLAG_EXCL)<0||!b2m.file){
        Com_Printf("HF241 INCOMPLETE label=%u reason=exclusive-open-failed no-retry\n",b2m.label);b2m.stopped=true;return false;
    }
    uint32_t header[32]={UINT32_C(0x52363446),1,128,B2M_RECORD_WORDS*4u,3,54,111,
        B2M_MAX_RECORDS,B2M_MAX_FRAMES,B2M_BYTE_CAP,86,12,32,283,162,5652,960,320,80,30,1,2,3,241,b2m.label,UINT32_C(0x03000000)};
    if(!b2m_write(header,sizeof(header))){b2m_close(4);return false;}
    Com_Printf("HF241 BEGIN label=%u file=%s samples=3 views=0,2 frame_cap=64 record_cap=128 byte_cap=770304 unpaused=1\n",b2m.label,name);
    return true;
}
// One request per process; neither label changes tracking or filter settings.
void R_B2HFSamplesArm(unsigned label)
{
    if (label<1 || label>2) return;
    if (b2m.label || b2m.stopped) {
        Com_Printf("HF241 request refused: already requested/terminal; relaunch for another capture.\n");
        return;
    }
    b2m.label=label;
    Com_Printf("HF241 ARMED label=%u; close console/resume gameplay to begin. Label does not set ET mode.\n",label);
}
void R_B2HFSamplesInterrupt(bool terminal)
{
    if (b2m.stopped || !b2m.label) return;
    if (b2m.file) b2m_close(8);
    else if (terminal) {
        b2m.stopped=true;
        Com_Printf("HF241 INCOMPLETE label=%u reason=interrupted-before-begin no-file no-retry\n",b2m.label);
    }
}
void R_B2HFSamplesFrameFinished(bool committed)
{
    if (!b2m.file || b2m.stopped) return;
    if (!committed || !B2_HFSamplesPlaying() || b2m.records!=2*b2m.frames) b2m_close(9);
    else if (b2m.records==B2M_MAX_RECORDS) b2m_close(1);
}
static void b2m_matrix(float out[4],const float m[16],const float v[4])
{
    for(unsigned r=0;r<4;r++){out[r]=0;for(unsigned c=0;c<4;c++)out[r]+=m[c*4+r]*v[c];}
}
static bool b2m_targets(const refdef_t *fd,const QVKUniformBuffer_t *u,uint32_t control[24])
{
    const b2_frame_t *f=fd->b2_frame;
    if(!f||f->view_count!=4||(fd->b2_eye!=0&&fd->b2_eye!=2)||u->width!=1948||u->height!=1563||
        u->current_gpu_slice_width!=u->width||u->pt_projection!=0||u->pt_swap_checkerboard!=0||
        u->taa_output_width<=0||u->taa_output_height<=0||u->taa_output_width>8192||u->taa_output_height>8192||
        u->flt_enable==0||u->flt_taa!=2||u->temporal_blend_factor!=0||u->pt_aperture!=0||
        !isfinite(u->sub_pixel_jitter[0])||!isfinite(u->sub_pixel_jitter[1])||
        fabsf(u->sub_pixel_jitter[0])>1||fabsf(u->sub_pixel_jitter[1])>1)return false;
    for(unsigned k=0;k<3;k++)if(f->views[0].origin[k]!=f->views[2].origin[k])return false;
    static const float x[3]={.5f,.98f,.5f};
    // Immutable R59 qualified middle-baseline frame6345,point1,focus world hit.
    // Keep its measured uncertainty in LANDMARK.json; do not snap to a plane.
    static const float landmark[4]={-1663.968295259746f,1580.714630358172f,4.105451501345874f,1.f};
    for(unsigned i=0;i<3;i++){
        float flat[2]={floorf(x[i]*u->width),floorf((i==0?.98f:.5f)*u->height)};
        float clip[4]={2*(flat[0]+.5f+u->sub_pixel_jitter[0])/u->width-1,
            2*(flat[1]+.5f+u->sub_pixel_jitter[1])/u->height-1,1,1};
        float focus[4],world[4],view[4],projected[4];
        if(i==1){memcpy(world,landmark,sizeof(world));}
        else{
            b2m_matrix(focus,f->views[2].invP,clip);focus[3]=0;
            b2m_matrix(world,f->views[2].invV,focus);
        }
        b2m_matrix(view,f->views[fd->b2_eye].V,world);
        b2m_matrix(projected,f->views[fd->b2_eye].P,view);
        bool finite=true;for(unsigned k=0;k<4;k++)finite=finite&&isfinite(projected[k]);
        uint32_t reason=0;
        if(!finite)reason=1u<<(17+i);
        else if(projected[3]<=0)reason=1u<<(11+i);
        if(reason){control[3]|=(1u<<(8+i))|reason;continue;}
        int p[2],o[2];
        for(unsigned k=0;k<2;k++){
            float dimension=(float)(k?u->height:u->width);
            float uv=(projected[k]/projected[3]+1)*.5f;
            float render=uv*dimension-.5f-u->sub_pixel_jitter[k];
            if(!isfinite(render)){reason=1u<<(17+i);break;}
            if(render<0||render>dimension-1){reason=1u<<(11+i);break;}
            p[k]=(int)floorf(render+.5f);
            float output=(p[k]+.5f+u->sub_pixel_jitter[k])*(k?u->taa_output_height:u->taa_output_width)/dimension-.5f;
            if(!isfinite(output)){reason=1u<<(17+i);break;}
            o[k]=(int)floorf(output+.5f);
            if(o[k]<0||o[k]>=(k?u->taa_output_height:u->taa_output_width)){reason=1u<<(11+i);break;}
        }
        if(reason){control[3]|=(1u<<(8+i))|reason;continue;}
        control[11+i*2]=(uint32_t)(p[0]/2+(((p[0]&1)==(p[1]&1))?0:u->width/2));
        control[12+i*2]=(uint32_t)p[1];control[17+i*2]=(uint32_t)o[0];control[18+i*2]=(uint32_t)o[1];
    }
    uint32_t collision=0;
    for(unsigned i=0;i<3;i++)for(unsigned j=i+1;j<3;j++){
        if(control[3]&((1u<<(8+i))|(1u<<(8+j))))continue;
        if((control[11+i*2]==control[11+j*2]&&control[12+i*2]==control[12+j*2])||
           (control[17+i*2]==control[17+j*2]&&control[18+i*2]==control[18+j*2]))collision|=(1u<<i)|(1u<<j);
    }
    control[3]|=(collision<<8)|(collision<<14);
    for(unsigned i=0;i<3;i++)if(control[3]&(1u<<(8+i))){
        control[11+i*2]=control[12+i*2]=control[17+i*2]=control[18+i*2]=UINT32_MAX;
    }
    return true;
}
static void b2m_barrier(VkCommandBuffer cmd,VkAccessFlags src,VkAccessFlags dst,VkPipelineStageFlags from,VkPipelineStageFlags to)
{
    VkBufferMemoryBarrier b={.sType=VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,.srcAccessMask=src,.dstAccessMask=dst,
        .srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,
        .buffer=qvk.buf_readback.buffer,.offset=offsetof(ReadbackBuffer,b1_tone_witness),.size=5620};
    vkCmdPipelineBarrier(cmd,from,to,0,0,NULL,1,&b,0,NULL);
}
static void b2m_prepare(VkCommandBuffer cmd,const refdef_t *fd,const QVKUniformBuffer_t *u)
{
    b2m.armed=false;b2m.tail_dirty=false;
    if(!fd)return;
    bool playing=B2_HFSamplesPlaying();
    if(b2m.file && !playing)b2m_close(8);
    bool requested=fd->b2_active&&!fd->b1_active&&b2m.label&&!b2m.stopped&&playing;
    if(!requested&&!b2m.tail_owned)return;
    _Static_assert(sizeof(ReadbackBuffer)==5652,"R51 extended readback ABI");
    _Static_assert(offsetof(ReadbackBuffer,b1_tone_witness)==32,"R48 retains feedback prefix");
    _Static_assert(sizeof(((ReadbackBuffer*)0)->b1_tone_witness)==1132,"R48 bounded tail");
    _Static_assert(offsetof(ReadbackBuffer,b2_lighting_witness)==1164,"R51 append only");
    _Static_assert(sizeof(((ReadbackBuffer*)0)->b2_lighting_witness)==648,"R51 three54-word slots");
    _Static_assert(offsetof(ReadbackBuffer,b2_hf_later)==1812,"R64 append offset");
    _Static_assert(sizeof(((ReadbackBuffer*)0)->b2_hf_later)==3840,"R64 bounded stage slots");
    _Static_assert(B2M_RECORD_WORDS*4u==6016u,"R64 record extent");
    _Static_assert(B2M_MAX_RECORDS==2u*B2M_MAX_FRAMES,"R64 full pairs");
    _Static_assert(256u+B2M_MAX_RECORDS*B2M_RECORD_WORDS*4u<=B2M_BYTE_CAP,"R64 footer reserved");
    b2m_barrier(cmd,VK_ACCESS_SHADER_WRITE_BIT|VK_ACCESS_TRANSFER_READ_BIT,VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT);
    vkCmdFillBuffer(cmd,qvk.buf_readback.buffer,32,5620,0);b2m.tail_owned=requested;b2m.tail_dirty=true;
    if(!requested){if(b2m.file)b2m_close(2);goto ready;}
    if(fd->b2_eye!=0&&fd->b2_eye!=2)goto ready;
    if(!fd->b2_frame){b2m_close(5);goto ready;}
    if(b2m.records>=B2M_MAX_RECORDS||(fd->b2_eye==0&&b2m.frames>=B2M_MAX_FRAMES)){b2m_close(9);goto ready;}
    if ((fd->b2_eye==0 && b2m.records!=2*b2m.frames) ||
        (fd->b2_eye==2 && (!b2m.frames || b2m.records!=2*b2m.frames-1)) ||
        (b2m.frames && ((uint32_t)u->current_frame_idx!=b2m.metadata[4]+(fd->b2_eye==0?1u:0u) ||
            fd->b2_frame->history_generation!=b2m.metadata[7]))) {b2m_close(9);goto ready;}
    if(!b2m_open())goto ready;
    memset(b2m.control,0,sizeof(b2m.control));
    b2m.control[0]=B2M_MAGIC;b2m.control[1]=fd->b2_eye;b2m.control[2]=(uint32_t)u->current_frame_idx;
    b2m.control[4]=fd->b2_frame->history_generation;
    b2m.control[6]=(uint32_t)u->width;b2m.control[7]=(uint32_t)u->height;
    b2m.control[9]=1;b2m.control[10]=3;
    bool route_supported=b2m_targets(fd,u,b2m.control);
    bool matrix_supported=b2m_pack_matrices(u,b2m.compact);
    bool supported=route_supported&&matrix_supported;
    if(!supported){memset(b2m.control+11,0,13*4);b2m.control[0]=0;}
    if(fd->b2_eye==0)b2m.frames++;
    b2m.control[5]=b2m.frames;
    memset(b2m.metadata,0,sizeof(b2m.metadata));uint32_t *m=b2m.metadata;
    m[0]=UINT32_C(0x52363452);m[1]=1;m[2]=B2M_RECORD_WORDS*4u;m[3]=b2m.records;
    m[4]=fd->b2_frame->logical_index;m[5]=fd->b2_eye;m[6]=m[4]&1u;m[7]=fd->b2_frame->history_generation;
    m[8]=Sys_Milliseconds();m[9]=b2m.first_ms;m[11]=b2m.previous_write_ms;
    uint64_t predicted=(uint64_t)fd->b2_frame->predicted_time_ns,duration=(uint64_t)fd->b2_frame->duration_ns;
    m[12]=(uint32_t)predicted;m[13]=(uint32_t)(predicted>>32);m[14]=(uint32_t)duration;m[15]=(uint32_t)(duration>>32);
    m[16]=u->width;m[17]=u->height;m[18]=u->prev_width;m[19]=u->prev_height;
    m[20]=u->taa_output_width;m[21]=u->taa_output_height;m[22]=fd->b2_frame->output_width[fd->b2_eye];m[23]=fd->b2_frame->output_height[fd->b2_eye];
    m[24]=u->temporal_history_valid;m[25]=u->flt_enable;m[26]=u->current_frame_idx;m[27]=u->current_gpu_slice_width;m[28]=u->pt_projection;m[29]=u->pt_swap_checkerboard;
    m[30]=b2m_bits(u->sub_pixel_jitter[0]);m[31]=b2m_bits(u->sub_pixel_jitter[1]);
    m[32]=b2m_bits(u->flt_min_alpha_color_hf);m[33]=b2m_bits(u->flt_min_alpha_moments_hf);m[34]=b2m_bits(u->inv_width);m[35]=b2m_bits(u->flt_show_gradients);
    m[36]=b2m_bits(u->temporal_blend_factor);m[37]=b2m_bits(u->pt_num_bounce_rays);m[38]=b2m_bits(u->flt_taa);m[39]=vkpt_fsr_is_enabled()?1:0;
    memcpy(m+40,u->V,64);memcpy(m+56,u->P,64);memcpy(m+72,u->invV,64);memcpy(m+88,u->invP,64);
    memcpy(m+104,u->V_prev,64);memcpy(m+120,u->P_prev,64);memcpy(m+136,u->invP_prev,64);
    memcpy(m+152,fd->b2_frame->views[2].P,64);memcpy(m+168,fd->b2_frame->views[2].invV,64);
    memcpy(m+184,fd->b2_frame->views[0].P,64);memcpy(m+200,fd->b2_frame->views[0].V,64);
    m[216]=b2m.frames;m[217]=b2m_bits(u->pt_aperture);m[218]=(route_supported?0u:1u)|(matrix_supported?0u:2u)|b2m.control[3]|UINT32_C(0x01000000)|
        (B2XR_DenoiserFoveated(fd->b2_frame)?UINT32_C(0x02000000):0u);
    m[219]=(uint32_t)u->prev_taa_output_width;m[220]=(uint32_t)u->prev_taa_output_height;m[221]=UINT32_MAX;
    m[222]=b2m_bits(u->flt_scale_lf);m[223]=b2m_bits(u->flt_scale_hf);m[224]=b2m_bits(u->flt_scale_spec);
    m[225]=b2m_bits(u->flt_scale_overlay);m[226]=b2m_bits(u->flt_fixed_albedo);
    b2m_barrier(cmd,VK_ACCESS_TRANSFER_WRITE_BIT,VK_ACCESS_TRANSFER_WRITE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT);
    vkCmdUpdateBuffer(cmd,qvk.buf_readback.buffer,32,sizeof(b2m.control),b2m.control);b2m.armed=true;
ready:
    b2m_barrier(cmd,VK_ACCESS_TRANSFER_WRITE_BIT,VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_SHADER_WRITE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
}
// R52 actual host dispatch/skip branch; no native action or renderer feedback.
static void b2m_bloom_selection(bool dispatch,int debug)
{
    if(!b2m.armed)return;
    b2m.metadata[221]=dispatch?((debug>=1&&debug<=3)?(uint32_t)debug+1u:1u):0u;
    b2m.metadata[227]=(uint32_t)debug;
}
static void b2m_before_tone(VkCommandBuffer cmd)
{
    if(b2m.armed)b2m_barrier(cmd,VK_ACCESS_SHADER_WRITE_BIT,VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_SHADER_WRITE_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}
static void b2m_before_copy(VkCommandBuffer cmd)
{
    if(b2m.tail_dirty)b2m_barrier(cmd,VK_ACCESS_TRANSFER_WRITE_BIT|VK_ACCESS_SHADER_WRITE_BIT,VK_ACCESS_TRANSFER_READ_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT);
}
static void b2m_finished(uint32_t eye)
{
    if(!b2m.armed||!b2m.file||b2m.metadata[5]!=eye)return;
    if(!B2_HFSamplesPlaying()){b2m_close(8);return;}
    uint32_t words[B2M_RECORD_WORDS]={0};ReadbackBuffer snapshot;uint32_t start=Sys_Milliseconds();
    VkResult result=vkpt_readback(&snapshot);uint32_t elapsed=Sys_Milliseconds()-start;
    b2m.max_snapshot_ms=max(b2m.max_snapshot_ms,elapsed);
    if(result!=VK_SUCCESS){b2m_close(6);return;}
    if(memcmp(snapshot.b1_tone_witness,b2m.control,sizeof(b2m.control))!=0){b2m_close(7);return;}
    memcpy(words,b2m.metadata,40*4);words[10]=elapsed;
    memcpy(words+40,b2m.compact,60*4);
    memcpy(words+100,b2m.metadata+216,3*4);
    memcpy(words+103,b2m.metadata+219,8*4);
    memcpy(words+111,snapshot.b1_tone_witness+11,12*4);
    memcpy(words+123,snapshot.b1_tone_witness+24,258*4);
    memcpy(words+381,snapshot.b2_lighting_witness,162*4);
    words[543]=b2m.metadata[227];
    memcpy(words+544,snapshot.b2_hf_later,sizeof(snapshot.b2_hf_later));
    start=Sys_Milliseconds();bool written=b2m_write(words,sizeof(words));elapsed=Sys_Milliseconds()-start;
    b2m.previous_write_ms=elapsed;b2m.max_write_ms=max(b2m.max_write_ms,elapsed);b2m.armed=false;
    if(!written){b2m_close(4);return;}b2m.records++;
    // Footer waits for the existing complete four-view client transaction.
}
#endif
