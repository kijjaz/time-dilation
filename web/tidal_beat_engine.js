class TidalBeatEngine {
    constructor() {
        this.audioCtx = null;
        this.isPlaying = false;
        this.timeMode = 'BeatMode'; // 'BeatMode' or 'CycleMode'
        this.bpm = 135;
        this.motifId = 0; // 0: 123, 1: 321, 2: 123 321, 3: Var 7, 4: Gopuchha, 5: Srotovaha, 6: Mridanga, 7: Euclidean
        this.euclideanPulses = 5;
        this.euclideanSteps = 8;
        this.currentBeatPos = 0;
        this.currentEvents = [];
        this.timerId = null;

        this.generateEvents();
    }

    initAudio() {
        if (!this.audioCtx) {
            this.audioCtx = new (window.AudioContext || window.webkitAudioContext)();
        }
        if (this.audioCtx.state === 'suspended') {
            this.audioCtx.resume();
        }
    }

    setMotif(id) {
        this.motifId = id;
        this.generateEvents();
    }

    setEuclideanParams(k, n) {
        this.euclideanPulses = k;
        this.euclideanSteps = n;
        if (this.motifId === 7) {
            this.generateEvents();
        }
    }

    // Bjorklund Euclidean Algorithm E(k, n)
    generateEuclidean(k, n) {
        let pattern = [];
        let counts = [];
        let remainder = [];
        let divisor = n - k;

        pattern.push(...Array(k).fill([1]));
        remainder.push(...Array(divisor).fill([0]));

        while (remainder.length > 1) {
            let count = Math.floor(pattern.length / remainder.length);
            counts.push(count);
            let newPattern = [];
            for (let i = 0; i < remainder.length; ++i) {
                newPattern.push([...pattern[i], ...remainder[i]]);
            }
            let leftOver = pattern.slice(remainder.length);
            pattern = newPattern;
            remainder = leftOver;
        }
        return pattern.flat(Infinity);
    }

    generateEvents() {
        this.currentEvents = [];
        let counts = [1, 2, 3];

        if (this.motifId === 1) counts = [3, 2, 1];
        else if (this.motifId === 2) counts = [1, 2, 3, 3, 2, 1];
        else if (this.motifId === 3) counts = [3, 3, 3, 2, 2, 2, 1, 1, 1, 3, 2, 1];
        else if (this.motifId === 4) counts = [5, 4, 3, 2, 1]; // Gopuchha Yati (Contracting)
        else if (this.motifId === 5) counts = [1, 2, 3, 4, 5]; // Srotovaha Yati (Expanding)
        else if (this.motifId === 6) counts = [1, 3, 5, 3, 1]; // Mridanga Yati (Hourglass)

        if (this.motifId === 7) {
            // Euclidean Mode
            const euc = this.generateEuclidean(this.euclideanPulses, this.euclideanSteps);
            let beatCursor = 0;
            const stepWidth = 16.0 / this.euclideanSteps;

            euc.forEach((hit, idx) => {
                if (hit === 1) {
                    this.currentEvents.push({
                        beatPosition: beatCursor,
                        durationBeats: stepWidth,
                        syllableIndex: idx % 6,
                        countValue: this.euclideanPulses
                    });
                }
                beatCursor += stepWidth;
            });
            return;
        }

        let beatCursor = 0;
        let sylIdx = 0;

        for (let count of counts) {
            for (let r = 0; r < count; ++r) {
                this.currentEvents.push({
                    beatPosition: beatCursor,
                    durationBeats: 1.0,
                    syllableIndex: sylIdx % 4,
                    countValue: count
                });
                beatCursor += 1.5; // Note + rest gap (-)
            }

            // Cadence phrase (Thakadimi)
            this.currentEvents.push({
                beatPosition: beatCursor,
                durationBeats: 2.0,
                syllableIndex: 5,
                countValue: count
            });
            beatCursor += 2.0;
            sylIdx++;
        }
    }

    mutatePattern() {
        for (let ev of this.currentEvents) {
            ev.syllableIndex = (ev.syllableIndex + Math.floor(Math.random() * 3)) % 6;
            ev.beatPosition = Math.max(0, ev.beatPosition + (Math.random() * 0.5 - 0.25));
        }
    }

    triggerPercussion(syllableIdx, time) {
        if (!this.audioCtx) return;

        const osc = this.audioCtx.createOscillator();
        const gain = this.audioCtx.createGain();

        const freqs = [85, 220, 110, 330, 440, 180];
        osc.frequency.setValueAtTime(freqs[syllableIdx % 6], time);

        gain.gain.setValueAtTime(0.4, time);
        gain.gain.exponentialRampToValueAtTime(0.001, time + 0.25);

        osc.connect(gain);
        gain.connect(this.audioCtx.destination);

        osc.start(time);
        osc.stop(time + 0.25);
    }

    triggerClick(time) {
        if (!this.audioCtx) return;

        const osc = this.audioCtx.createOscillator();
        const gain = this.audioCtx.createGain();

        osc.type = 'triangle';
        osc.frequency.setValueAtTime(1200, time);

        gain.gain.setValueAtTime(0.15, time);
        gain.gain.exponentialRampToValueAtTime(0.001, time + 0.05);

        osc.connect(gain);
        gain.connect(this.audioCtx.destination);

        osc.start(time);
        osc.stop(time + 0.05);
    }

    start() {
        this.initAudio();
        this.isPlaying = true;
        this.currentBeatPos = 0;

        let nextBeatTime = this.audioCtx.currentTime;

        const schedule = () => {
            if (!this.isPlaying) return;

            const secondsPerBeat = 60.0 / this.bpm;
            const lookahead = 0.1;

            while (nextBeatTime < this.audioCtx.currentTime + lookahead) {
                // Check event triggers
                for (let ev of this.currentEvents) {
                    let evBeat = ev.beatPosition;
                    if (this.timeMode === 'CycleMode') {
                        evBeat = evBeat % 8.0;
                    }

                    if (Math.abs(this.currentBeatPos - evBeat) < 0.1) {
                        this.triggerPercussion(ev.syllableIndex, nextBeatTime);
                    }
                }

                // Click pulse
                this.triggerClick(nextBeatTime);

                this.currentBeatPos = (this.currentBeatPos + 0.5) % 16.0;
                nextBeatTime += secondsPerBeat * 0.5;
            }

            this.timerId = setTimeout(schedule, 25);
        };

        schedule();
    }

    stop() {
        this.isPlaying = false;
        if (this.timerId) clearTimeout(this.timerId);
    }
}
