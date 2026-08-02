document.addEventListener('DOMContentLoaded', () => {
    // Startup Splash Screen Controller
    const splashOverlay = document.getElementById('splashOverlay');
    if (splashOverlay) {
        // Trigger smooth 3-second auto fade out
        const fadeTimer = setTimeout(() => {
            splashOverlay.classList.add('fade-out');
        }, 100);

        // Click anywhere to fade out instantly/quickly (200ms)
        splashOverlay.addEventListener('click', () => {
            clearTimeout(fadeTimer);
            splashOverlay.classList.add('fast-fade');
        });
    }

    const engine = new TidalBeatEngine();

    const canvas = document.getElementById('beatCanvas');
    const ctx = canvas.getContext('2d');

    const modeBtn = document.getElementById('modeBtn');
    const playBtn = document.getElementById('playBtn');
    const bpmSlider = document.getElementById('bpmSlider');
    const bpmVal = document.getElementById('bpmVal');
    const cycleStatus = document.getElementById('cycleStatus');
    const exportBtn = document.getElementById('exportBtn');
    const motifBtns = document.querySelectorAll('.btn-motif');

    // Mode Toggle
    modeBtn.addEventListener('click', () => {
        if (engine.timeMode === 'BeatMode') {
            engine.timeMode = 'CycleMode';
            modeBtn.textContent = 'CYCLE MODE (TidalCycles)';
            cycleStatus.textContent = 'CYCLE MODE: NORMALIZED REPETITIVE TIME';
        } else {
            engine.timeMode = 'BeatMode';
            modeBtn.textContent = 'BEAT MODE (Tidal 2.0)';
            cycleStatus.textContent = 'BEAT MODE: ABSOLUTE TIME CONTINUUM';
        }
    });

    // Play / Stop Toggle
    playBtn.addEventListener('click', () => {
        if (engine.isPlaying) {
            engine.stop();
            playBtn.textContent = 'START PERMUTATIONS';
            playBtn.classList.remove('active');
        } else {
            engine.start();
            playBtn.textContent = 'STOP PERMUTATIONS';
            playBtn.classList.add('active');
        }
    });

    // Tempo Control
    bpmSlider.addEventListener('input', (e) => {
        engine.bpm = parseFloat(e.target.value);
        bpmVal.textContent = engine.bpm;
    });

    // Motif Selection
    motifBtns.forEach(btn => {
        btn.addEventListener('click', (e) => {
            motifBtns.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            const motifId = parseInt(btn.getAttribute('data-motif'));
            engine.setMotif(motifId);
        });
    });

    const eucKSlider = document.getElementById('eucKSlider');
    const eucKVal = document.getElementById('eucKVal');
    const eucNSlider = document.getElementById('eucNSlider');
    const eucNVal = document.getElementById('eucNVal');
    const mutateBtn = document.getElementById('mutateBtn');

    // Euclidean Sliders
    eucKSlider.addEventListener('input', (e) => {
        const k = parseInt(e.target.value);
        eucKVal.textContent = k;
        engine.setEuclideanParams(k, parseInt(eucNSlider.value));
    });

    eucNSlider.addEventListener('input', (e) => {
        const n = parseInt(e.target.value);
        eucNVal.textContent = n;
        engine.setEuclideanParams(parseInt(eucKSlider.value), n);
    });

    // Mutate Button
    mutateBtn.addEventListener('click', () => {
        engine.mutatePattern();
    });

    // Render Beat Matrix Canvas
    function renderCanvas() {
        const width = canvas.width;
        const height = canvas.height;

        ctx.fillStyle = '#0d1321';
        ctx.fillRect(0, 0, width, height);

        const totalBeats = 16;
        const beatWidth = width / totalBeats;

        // Draw Beat Subdivision Lines
        for (let b = 0; b < totalBeats; ++b) {
            const bx = b * beatWidth;
            ctx.strokeStyle = (b % 4 === 0) ? '#334155' : '#1e293b';
            ctx.lineWidth = (b % 4 === 0) ? 2 : 1;

            ctx.beginPath();
            ctx.moveTo(bx, 0);
            ctx.lineTo(bx, height);
            ctx.stroke();

            ctx.fillStyle = '#64748b';
            ctx.font = '10px "Fira Code", monospace';
            ctx.fillText((b + 1).toString(), bx + 4, 16);
        }

        // Draw Solkattu Events
        const sylColours = ['#00ff66', '#ffb000', '#a855f7', '#00e5ff', '#f43f5e', '#e11d48'];
        const sylNames = ['Tha', 'Dhi', 'Thom', 'Nam', 'Tarikita', 'Thakadimi'];

        for (let ev of engine.currentEvents) {
            const ex = (ev.beatPosition / totalBeats) * width;
            const ew = (ev.durationBeats / totalBeats) * width - 3;
            const ey = 35 + (ev.syllableIndex % 4) * 50;
            const eh = 40;

            if (ex + ew <= width) {
                ctx.fillStyle = sylColours[ev.syllableIndex % 6];
                ctx.globalAlpha = 0.85;
                ctx.beginPath();
                ctx.roundRect(ex, ey, ew, eh, 6);
                ctx.fill();

                ctx.globalAlpha = 1.0;
                ctx.fillStyle = '#000000';
                ctx.font = 'bold 12px "Fira Code", monospace';
                ctx.textAlign = 'center';
                ctx.fillText(`${sylNames[ev.syllableIndex % 6]} (${ev.countValue})`, ex + ew / 2, ey + 24);
            }
        }

        // Draw Live Beat Cursor
        if (engine.isPlaying) {
            const cursorX = (engine.currentBeatPos / totalBeats) * width;
            ctx.strokeStyle = '#ffffff';
            ctx.lineWidth = 3;
            ctx.beginPath();
            ctx.moveTo(cursorX, 0);
            ctx.lineTo(cursorX, height);
            ctx.stroke();
        }

        requestAnimationFrame(renderCanvas);
    }

    renderCanvas();
});
