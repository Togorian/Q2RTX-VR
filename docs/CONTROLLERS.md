# Controller guide

Factory defaults select **Togorian**, right-handed. In **Options → VR Controller Settings**, choose handedness and Togorian, Quest-style or Custom. These are application layouts, not proof that a runtime exposes every button. Primary/secondary means X/Y on a Touch-profile left controller and A/B on the right. Index-profile controllers use A/B on both hands. With the appropriate runtime mapping, Sony PS VR2 labels are Square/Triangle and Cross/Circle. Use the physical hand and displayed role when labels differ.

| Action | Togorian, right-handed | Togorian, left-handed |
|---|---|---|
| Move / turn | Left stick / right stick | Right stick / left stick |
| Fire / respawn | Right trigger | Left trigger |
| Jump | Right primary | Right primary |
| Crouch | Left grip | Right grip |
| Teleport hold/release | Left trigger | Right trigger |
| Use | Left primary | Right secondary |
| Weapon / item selector | Right grip | Left grip |
| Recenter | Right stick click | Left stick click |
| Menu / back shortcut | Right secondary | Left secondary |
| Additional menu shortcut during films | Left secondary | Left secondary |
| Interruption resume | Left secondary | Right secondary |
| Walk / run toggle | Left stick click | Right stick click |
| Mission / inventory / closed cycle during play | Left secondary, fresh presses | Left secondary, fresh presses |

In both Togorian layouts, the shared left-secondary input cycles mission, inventory and closed overlays during play; its additional-menu role applies during films. In the left-handed layout it also shares Menu/Back: inside menus it goes back, while during play the cycle takes priority. Use an available hardware Menu input to open menus in that shared-input case. The right-handed right-secondary Menu/Back shortcut remains separate. Hardware Menu is separate from these editable actions; a runtime-reserved system button is not made available by this application.

Quest-style keeps move, turn, fire, teleport, selector, recenter, resume and walk/run in the same dominant/off-hand roles. Jump moves to dominant secondary, crouch to dominant primary, Items to off-hand grip, stock inventory to off-hand primary, and mission helper to off-hand secondary. Use, face Menu shortcuts and the cycle are unbound. Use the available hardware Menu route.

In menus, either stick navigates and either primary confirms. The fixed Menu route depends on the active runtime. The application does not override a reserved right Meta/system button.

Only **Custom** unlocks action rows. **Apply to Custom settings** copies a preset into pending Custom; select Custom to edit it, then **Apply** to save. Cancel/Back discards pending binding and handedness edits. Haptics applies immediately and saves across presets. Conflicting assignments are checked for both handednesses.

Keyboard fallback defaults: Home recentres; End switches controllers / keyboard and mouse. In the existing **Options → Action Key Bindings** list, change **Recentre view** or **Switch controllers / keyboard and mouse** using the normal key-binding control. Enter chooses a key; Backspace clears its binding. The saved recentre binding also works in menus and cutscenes without skipping the film. Text fields, console input and key capture keep their normal input. Switching input mode retains its existing gameplay readiness requirements. F2 opens Save, F3 opens Load, F6 quicksaves and F9 quickloads. The optional reset configuration runs only when explicitly requested; startup does not reset a player's keys.

These tables describe defaults; saved settings can differ. See [KNOWN-ISSUES.md](KNOWN-ISSUES.md).

Current clean/reset defaults use smooth turning at 360 degrees/second and teleport-only vignette. Snap turning and movement/turn vignette remain selectable. Existing saved preferences are retained on ordinary startup; Reset to Defaults explicitly restores these values. One absent controller does not invalidate the other hand’s available buttons/stick. Each returning action requires its own neutral observation; fire also requires valid aiming and release/repress after loss. Fresh valid-but-untracked aiming may continue for at most 200 ms after full tracking; invalid or stale data is refused. Controller focus follows OpenXR FOCUSED; keyboard/mouse follows desktop focus. Both modes render in the headset. See [known limitations](KNOWN-ISSUES.md).

## Controller profiles

Supported headsets are Pimax Dream Air, Sony PS VR2 and Meta Quest 3. The active OpenXR runtime chooses the controller profile; a headset name does not guarantee a particular button mapping.

| Runtime profile | Available application bindings |
|---|---|
| Oculus Touch (`/interaction_profiles/oculus/touch_controller`) | Aim/grip pose, triggers, grips, sticks/clicks, X/Y left and A/B right; left Menu only. Right Meta remains reserved by the runtime. |
| Valve Index (`/interaction_profiles/valve/index_controller`) | Aim/grip pose, triggers, grips, sticks/clicks and A/B on both hands. The application does not bind the system button; use the configured face-button Menu route. |
| HP mixed reality (`/interaction_profiles/hp/mixed_reality_controller`) | When the runtime advertises the HP extension: aim/grip pose, triggers, grips, sticks/clicks, X/Y left and A/B right, Menu on both hands. |
| Khronos simple (`/interaction_profiles/khr/simple_controller`) | Limited fallback: aim/grip pose, Select as primary/confirm, and Menu on either hand. Haptics is optional. No analogue trigger, squeeze, stick, stick click or secondary button is invented. |

The simple profile cannot provide full controller locomotion, weapon controls or stick menu navigation. Use keyboard/mouse for those controls and for deliberate resume when the assigned button is absent. The start panel identifies a missing supported profile or a limited simple profile separately for each hand. A missing hand does not disable the other hand. The available actions depend on the active runtime.

The **Pause key on the keyboard** resumes from a hold and starts play from the **Preparing play** panel. **End** switches controllers / keyboard and mouse only after play has started. With no controllers, start a game from the menu using the keyboard, then press End. That input-mode choice is remembered at the next launch; saved key bindings can differ.
