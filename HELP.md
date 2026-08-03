===============================================================================
                       TIME DILATION DAW WORKSTATION
                             VERSION 4.0 MANUAL
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


5. RELATIVISTIC TIME OBJECT SYSTEM
-------------------------------------------------------------------------------
Time context is passed along purple time ports (NodePortType::Time) using time
dilation factor gamma.

A. Time Dilation Math:
   - Gamma = 1.0  : Standard real-time progression (1 second per second).
   - Gamma > 1.0  : Dilated acceleration (time moves fast, pitch shifts up).
   - Gamma < 1.0  : Temporal deceleration / redshift (time moves slow).
   - Gamma = 0.0  : Gravitational stasis (time holds perfectly still).
   - Gamma = -1.0 : Retrograde causality (time moves in reverse).

B. Core Time Nodes:
   - [time.warp~]     : Continuous time speed scaler (0.1x to 10.0x).
   - [time.retro~]    : Temporal reversal engine (-1.0x).
   - [time.stasis~]   : Gravitational freeze gate.
   - [time.quantize~] : Snaps continuous gamma to metric grid steps (1/16, 1/8).
   - [time.metro~]    : Dilated metronome tick generator.
   - [time.transport] : Relativistic master transport hub.
   - [time.scope]     : Telemetry visualizer for local time (t) and gamma.
   - [time.future~]    : Future Lookahead Causality Offset Engine (shifts project horizon).


11. FUTURE LOOKAHEAD CAUSALITY OFFSET ENGINE ([time.future~])
-------------------------------------------------------------------------------
When a node requires signals from the future (e.g. 1.0 second lookahead):
   - The DAW automatically applies a global causality delay (+1.0s) to all standard
     nodes across the workspace.
   - The node with [time.future~ 1.0s] reads directly from the live un-delayed
     input stream, allowing future prediction while shifting the observer horizon.


12. RELATIVISTIC SEQUENCER SUITE
-------------------------------------------------------------------------------
   - [seq] / [step]   : Multi-step pattern sequencer. Accepts floats, MIDI notes
                        ("C4 E4 G4 B4"), and pattern strings.
   - [euclid k n]     : Euclidean rhythm generator distributing k pulses over n steps.
   - [markov]         : Generative Markov chain probability sequencer for dynamic
                        stochastic melody & rhythm generation.
   - [tidal] / [tidal~]: TidalCycles-style mini-notation pattern sequencer parsing
                        nested Euclidean subdivisions e.g. "60 [62 64] 65 [67 69 71]",
                        rests ("~"), note names ("c4", "eb4"), and speed multipliers.


13. UNIVERSAL timeIn PORT STANDARD
-------------------------------------------------------------------------------
EVERY single node object in the engine (audio, control, math, delay, filter, table,
expression, bang, sequencers, out~) includes a purple timeIn port as Inlet 0.
Connecting a time dilation factor (gamma) to any node dynamically scales its DSP
clock and processing rate (t -> gamma * t).


6. WIRELESS GLOBAL SIGNAL TAPPING (tap())
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


7. DUAL PERSPECTIVE GUARANTEE
-------------------------------------------------------------------------------
Every node features two full perspectives:
1. Top-Down Visual Inspector : Sliders, ranges, [TAP] buttons, +MOD INLETS.
2. Bottom-Up Code Math       : C++ DSP math expressions ($v1, $v2, $t, params).


8. WORKSTATION KEYBOARD SHORTCUTS
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


9. MULTI-INSTANCE RELATIVISTIC TIMELINE ([timeline])
-------------------------------------------------------------------------------
The DAW supports instantiating multiple independent arrangement timelines
directly on the node canvas ([timeline] object).

A. Features:
   - Multi-Track Deck: Houses Audio Tracks, MIDI Tracks, and Time Dilation Tracks.
   - Transport Sync: Connects to [time.transport] for synchronized playhead control.
   - Relativistic Time Inlet (timeIn): Time-warps, accelerates, or freezes playback.
   - Multi-Channel Outlets: Outputs stereo audio (outL~, outR~), MIDI control streams,
     and automated time dilation curves (dilationOut).

B. Arrangement Editor View:
   - Double-clicking a [timeline] card opens the interactive multi-track Arrangement View.
   - Track headers include [REC], [MUTE], [SOLO], [VOL] controls.
   - Displays real-time audio waveforms, MIDI note grids, and time dilation curves.


10. PROJECT BUNDLE & ASSET MANAGEMENT (.tdaw)
-------------------------------------------------------------------------------
Time Dilation DAW uses a self-contained Project Directory Bundle structure:

  MyProject.tdaw/
  ├── project.xml       # Full graph topology, nodes, connections & parameters
  ├── Assets/
  │   ├── Audio/        # Recorded and imported WAV audio clips
  │   └── MIDI/         # Recorded MIDI pattern streams
  └── Cache/            # Transient waveform overviews & render cache

A. Transient Cache & Save Migration Workflow:
   - Live recorded audio buffers are cached in a temporary system cache folder.
   - Clicking [Save Project] (Cmd+S) creates the .tdaw folder, copies all cache audio
     and MIDI files into Assets/Audio/ and Assets/MIDI/, and saves project.xml.
   - Opening a project (Cmd+O) automatically resolves relative asset paths
     (Assets/Audio/sample.wav), ensuring 100% project portability.
  - Shift + Drag       : Marquee Rubberband Selection.

===============================================================================
                  END OF TIME DILATION DAW MANUAL (HELP.md)
===============================================================================
