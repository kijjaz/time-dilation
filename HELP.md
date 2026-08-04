===============================================================================
                       TIME DILATION DAW WORKSTATION
                             VERSION 0.0.1 MANUAL
===============================================================================

1. EXECUTIVE OVERVIEW
-------------------------------------------------------------------------------
Time Dilation DAW is a Relativistic Modular Audio Workstation built with native
C++20 and JUCE 7. It unifies visual block diagram patching (Top-Down) with 
authentic C++ DSP expression coding (Bottom-Up).

Every node in the workspace operates on its own local coordinate time clock,
inheriting or transforming time dilation factors (gamma) dynamically.


2. AUDIO ENGINE SAFETY GUARANTEE
-------------------------------------------------------------------------------
* SAFE LAUNCH: The application unconditionally launches with AUDIO: OFF (SAFE).
  You must manually toggle the AUDIO power button in the top control bar to 
  start audio graph execution.

* FEEDBACK LOOP STABILITY: All audio connections forming feedback loops 
  automatically route through a 1-block delay history buffer (previousBlockBuffer),
  guaranteeing thread safety, preventing sample rate buffer lockups, and keeping
  DSP feedback loops 100% stable.


3. SYMBOL MESSAGES & TABLE METHOD SYSTEM
-------------------------------------------------------------------------------
Nodes in Time Dilation DAW receive simple symbol messages on their control inlets 
to invoke methods or update parameters dynamically.

A. [table] Object Methods:
   - set <idx> <val> : Writes sample value <val> at integer index <idx> in table.
   - get <idx>       : Reads sample value at index <idx> and outputs float value.
   - normalize       : Finds peak absolute sample (M = max(|x_i|)) and scales all
                       samples by 1.0 / M so peak amplitude equals 1.0 (0 dBFS).
   - clear           : Zeroes out all samples in table memory buffer.
   - resize <size>   : Re-allocates table memory size in samples.
   - fillSine        : Fills table memory with 1 full period of Sine wave.

B. Comprehensive Symbol Command Table Across Objects:

   [time.transport]
     - start / play  : Start transport timeline.
     - stop          : Stop transport and reset to beat 0.0.
     - pause         : Freeze transport at current beat.
     - record        : Toggle audio/MIDI recording mode.
     - seek <beat>   : Seek transport position to specified beat (e.g. "seek 16").
     - bpm <val>     : Set transport tempo in BPM (e.g. "bpm 135").
     - loop <0|1>    : Enable/disable transport loop.

   [sampler~]
     - start / play  : Trigger sample playback from start.
     - stop          : Stop sample playback immediately.
     - record        : Record live audio into RAM buffer.
     - read <file>   : Load WAV/FLAC audio file into buffer (e.g. "read drum.wav").
     - write <file>  : Export buffer memory to disk file.
     - clear         : Erase buffer memory to silence.
     - normalize     : Scale buffer peak amplitude to 1.0 (0 dBFS).

   [osc~]
     - reset         : Reset oscillator phase to zero (0.0 rad).
     - set <freq>    : Set oscillator frequency in Hz.
     - sine          : Switch oscillator waveform to Sine.
     - saw           : Switch oscillator waveform to Sawtooth.
     - square        : Switch oscillator waveform to Square.
     - triangle      : Switch oscillator waveform to Triangle.

   [table]
     - set <idx> <val>: Write sample value at index <idx>.
     - get <idx>     : Read sample value at index <idx>.
     - normalize     : Normalize peak amplitude to 1.0 (0 dBFS).
     - clear         : Zero out table memory.
     - resize <size> : Allocate table buffer sample length.
     - fillSine      : Fill table with 1 cycle of Sine wave.

   [filter~]
     - lowpass       : Set filter mode to Low-Pass.
     - highpass      : Set filter mode to High-Pass.
     - bandpass      : Set filter mode to Band-Pass.
     - bypass <0|1>  : Toggle filter bypass state.

   [delay~]
     - clear         : Flush internal delay buffer memory.
     - delay <ms>    : Set delay time in milliseconds.

   [out~]
     - mute          : Silence master output.
     - unmute        : Restore master output.
     - reset         : Reset VU peak and RMS meters.

   [bang]
     - bang          : Force emit a control pulse down all connected outlets.


4. MESSAGE PROCESSING & WIRELESS BUS OBJECTS
-------------------------------------------------------------------------------
   - [message] / [msg] : Text box symbol message box with $(...) math expressions.
   - [send busName]    : Wireless symbol message transmitter (omits patch cables).
   - [receive busName] : Wireless symbol message receiver.
   - [route val1 val2] : Routes incoming symbol selectors to matching outlets.
   - [spigot]          : Symbol message gate (pass/block control signals).
   - [delay ms]        : Delays symbol message pulses by specified milliseconds.


5. RELATIVISTIC TIME OBJECT SYSTEM & INTER-TIME MODULATION
-------------------------------------------------------------------------------
Time context is passed along purple time ports (NodePortType::Time) using time
dilation factor gamma (gamma = dtau / dt).

A. Time Dilation States:
   - Gamma = 1.0  : Standard real-time progression (1 second per second).
   - Gamma > 1.0  : Dilated acceleration (time moves fast, pitch shifts up).
   - Gamma < 1.0  : Temporal deceleration / redshift (time moves slow).
   - Gamma = 0.0  : Gravitational stasis (time holds perfectly still).
   - Gamma = -1.0 : Retrograde causality (time moves in reverse).

B. Core Relativistic Time Nodes & Mathematical Suite:
   - [time.warp~]       : Continuous time speed scaler (0.1x to 16.0x).
   - [time.retro~]      : Temporal reversal engine (-1.0x).
   - [time.stasis~]     : Gravitational freeze gate.
   - [time.quantize~]   : Snaps continuous gamma to metric grid steps (1/16, 1/8).
   - [time.metro~]      : Dilated metronome tick generator (outputs gamma LFO).
   - [time.math~]       : Time Signal Combiner (Add, Multiply, Min, Max, Mix).
   - [time.scale~]      : Time Dilation Signal Scaler & Offset Shifter.
   - [time.filter~]     : Gravitational inertia slew filter (smooths time jumps).
   - [time.boost~]      : Relativistic Velocity Boost & Einstein Doppler Factor Scaling.
   - [time.noise~]      : Relativistic Stochastic Temporal Jitter & Brownian Drift Generator.
   - [time.samplehold~] : Time Dilation Sample & Hold (samples continuous gamma on trigger).
   - [time.invert~]     : Reciprocal Time Dilation (1/gamma) for exact time un-warping.
   - [time.logic~]      : Relativistic Time Comparator & Gate (GreaterThan, Stasis, Max, Min).
   - [time.delay~]      : Time Dilation Signal Delay Line (Delays gamma signal propagation).
   - [audio2time~]      : Converts audio waveform amplitude into gamma time signal.
   - [time2audio~]      : Converts gamma time signal into audible audio stream.

C. Port Type Compatibility & Conversion Feedback:
   The workspace enforces signal compatibility between ports:
   - Time (Purple) <-> Time (Purple) : Direct connection allowed.
   - Time (Purple) -> Control (Amber) : Direct connection allowed (gamma scalar modulation).
   - Audio~ (Cyan) -> Time (Purple)   : Requires [audio2time~] or [a2t~] converter!
   - Time (Purple) -> Audio~ (Cyan)   : Requires [time2audio~] or [t2a~] converter!
   - Audio~ (Cyan) -> Control (Amber) : Requires [env~] or [snapshot~] converter!
   
   If incompatible ports are patched, a notification banner immediately alerts the user with converter tips!

D. Inter-Time Dilation Operations & Cascades:
   Time objects can feed directly into the timeIn (Inlet 0) or modulation inlets
   of other time objects!
   
   Example 1: Hierarchical Cascading Time Dilation
     [time.metro~ rate=1.5] ---> timeIn [time.warp~ dilationGamma=2.0]
     Result: The LFO output of [time.metro~] modulates the speed of [time.warp~],
     creating a breathing relativistic time warper that accelerates and decelerates
     all downstream DSP nodes dynamically.

   Example 2: Relativistic Lorentz Velocity Composition ([time.math~])
     [time.metro~ gamma1] --\
                            ---> [time.math~ mode=1 (Lorentz)] ---> timeIn [osc~]
     [time.warp~  gamma2] --/
     Result: Combines two independent time dilation fields using the authentic
     Lorentz boost addition formula: gamma_comp = 1 + ((v1 + v2) / (1 + v1*v2/c^2)).


6. TIME & AUDIO SIGNAL INTERCHANGE
-------------------------------------------------------------------------------
In Time Dilation DAW, time signals and audio signals can seamlessly interchange:
   - Direct Connection: Any Time outlet connected to audio nodes (*~, expr~,
     filter~, spectrometer~) streams its frame-by-frame gamma value as audio samples.
   - [audio2time~] : Converts audio amplitude into gamma time dilation:
                     gamma = offset + depth * audioIn
   - [time2audio~] : Converts gamma time signal into audible audio buffer:
                     audioOut = offset + scale * (gamma - 1.0)


7. CONTROL UI & INTERACTOR OBJECT SUITE
-------------------------------------------------------------------------------
   - [slider]         : Interactive control slider with integer mode (isInteger=1),
                        min, max, and offset parameters.
   - [toggle] / [tgl] : 0 / 1 toggle switch object.
   - [bang] / [b]     : Control pulse trigger button.
   - [bang~] / [b~]   : Audio-rate single-sample impulse spike generator.
   - [counter] / [cnt]: Discrete step counter with integer snapping (isInteger=1),
                        min, max, step, and offset.


8. MULTI-TRACK RELATIVISTIC ARRANGEMENT TIMELINE ([timeline])
-------------------------------------------------------------------------------
The DAW supports instantiating multi-track arrangement timelines directly on the
node canvas ([timeline] object).

A. Track Types Supported:
   - Audio Tracks     : Houses audio clips and stem recordings.
   - MIDI Tracks      : Houses MIDI note event grids and velocity sequences.
   - Time Dilation    : Houses continuous relativistic time dilation curves gamma(t).
   - Control Auto     : Houses control parameter and event value automation.

B. Transport & Relativistic Control:
   - Features a purple timeIn port to time-warp the entire arrangement timeline.
   - Multi-channel outlets: stereo audio (outL~, outR~), MIDI streams, and
     automated time dilation curves (dilationOut).


9. WIRELESS GLOBAL SIGNAL TAPPING (tap())
-------------------------------------------------------------------------------
Any node formula or text field can wirelessly tap signals from any node in 
the workspace without physical cables.

Syntax:
  - tap(node_id)             : Taps node by integer ID (e.g. tap(2)).
  - tap("node_label")        : Taps node by custom label (e.g. tap("osc~ 440")).
  - tap("node_label.param")  : Taps node parameter (e.g. tap("filter~.cutoff")).

1-Click UX:
  Clicking the [TAP] button on any parameter slider in the Top-Down Inspector
  copies tap("node_label.param") to the clipboard for instant pasting.


10. DUAL PERSPECTIVE GUARANTEE
-------------------------------------------------------------------------------
Every node features two full perspectives:
1. Top-Down Visual Inspector : Sliders, ranges, [TAP] buttons, +MOD INLETS.
2. Bottom-Up Code Math       : C++ DSP math expressions ($v1, $v2, $t, params).


11. WORKSTATION KEYBOARD SHORTCUTS
-------------------------------------------------------------------------------
  - Spacebar           : Toggle Global Audio Engine Power (ON / OFF).
  - N / Double-Click   : Open Node Creation Search Palette.
  - Cmd + S            : Save Project Bundle (.tdaw folder & project.xml).
  - Cmd + O            : Open Project Bundle (.tdaw project folder).
  - Cmd + A            : Select All Nodes.
  - Cmd + C / Cmd + V  : Copy / Paste Selected Nodes.
  - Cmd + D            : Duplicate Selected Nodes.
  - Cmd + Z            : Undo Action.
  - Cmd + Shift + Z    : Redo Action.
  - Delete / Backspace : Delete Selected Nodes or Connections.
  - Shift + Drag       : Marquee Rubberband Selection.


12. PROJECT BUNDLE & ASSET MANAGEMENT (.tdaw)
-------------------------------------------------------------------------------
Time Dilation DAW uses a self-contained Project Directory Bundle structure:

  MyProject.tdaw/
  ├── project.xml       # Full graph topology, nodes, connections & parameters
  ├── Assets/
  │   ├── Audio/        # Recorded and imported WAV audio clips
  │   └── MIDI/         # Recorded MIDI pattern streams
  └── Cache/            # Transient waveform overviews & render cache

===============================================================================
                  END OF TIME DILATION DAW MANUAL (HELP.md)
===============================================================================
