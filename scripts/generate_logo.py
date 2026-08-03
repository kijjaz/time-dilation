import math
import random
import colorsys
import os

class Particle:
    def __init__(self, x, y, vx, vy, radius, particle_type="standard", mass=1.0):
        self.x = x
        self.y = y
        self.vx = vx
        self.vy = vy
        self.radius = radius
        self.particle_type = particle_type # "micro", "standard", "medium", "macro"
        self.mass = mass
        v_init = math.sqrt(vx**2 + vy**2)
        self.history = [(x, y, v_init)] # Store (x, y, velocity)
        self.color_seed = random.uniform(-0.15, 0.15)

def run_pure_particle_simulation(num_steps=160, width=1000, height=600):
    random.seed(2026) # Deterministic physics seed

    bh_x, bh_y = 440, 300
    bh_mass = 16000.0
    r_event = 70.0
    r_photon = 92.0

    particles = []

    # 1. Macro Relativistic Time Nodes (r: 8.0 - 14.0px, count: 28)
    for _ in range(28):
        sx = random.uniform(-100, 280)
        sy = random.triangular(70, 530, 300)
        vx = random.uniform(2.8, 4.2)
        vy = random.uniform(-0.5, 0.5)
        rad = random.uniform(8.0, 14.0)
        mass = rad ** 1.8
        p = Particle(sx, sy, vx, vy, rad, particle_type="macro", mass=mass)
        particles.append(p)

    # 2. Medium Energy Clusters (r: 4.0 - 7.5px, count: 120)
    for _ in range(120):
        sx = random.uniform(-120, 340)
        sy = random.triangular(50, 550, 300)
        vx = random.uniform(3.2, 4.8)
        vy = random.uniform(-0.5, 0.5)
        rad = random.uniform(4.0, 7.5)
        mass = rad * 1.5
        p = Particle(sx, sy, vx, vy, rad, particle_type="medium", mass=mass)
        particles.append(p)

    # 3. Standard Time Grains (r: 1.5 - 3.5px, count: 450)
    for _ in range(450):
        sx = random.uniform(-140, 380)
        sy = random.triangular(30, 570, 300)
        vx = random.uniform(3.5, 5.5)
        vy = random.uniform(-0.4, 0.4)
        rad = random.uniform(1.5, 3.5)
        mass = rad * 0.8
        p = Particle(sx, sy, vx, vy, rad, particle_type="standard", mass=mass)
        particles.append(p)

    # 4. Micro Quantum Specs (r: 0.4 - 1.2px, count: 600)
    for _ in range(600):
        sx = random.uniform(-160, 400)
        sy = random.triangular(10, 590, 300)
        vx = random.uniform(3.8, 6.2)
        vy = random.uniform(-0.4, 0.4)
        rad = random.uniform(0.4, 1.2)
        mass = 0.2
        p = Particle(sx, sy, vx, vy, rad, particle_type="micro", mass=mass)
        particles.append(p)

    dt = 0.42
    macro_particles = [p for p in particles if p.particle_type in ("macro", "medium")]

    # Physics Integration
    for step in range(num_steps):
        for p in particles:
            dx_bh = bh_x - p.x
            dy_bh = bh_y - p.y
            dist_bh = math.sqrt(dx_bh**2 + dy_bh**2)

            if dist_bh > r_event * 0.75:
                f_bh = bh_mass / (dist_bh**2 + 110.0)
                tangent_x = -dy_bh / dist_bh
                tangent_y = dx_bh / dist_bh
                swirl = (240.0 / (dist_bh + 40.0)) if dist_bh < 300 else 0

                fx = (dx_bh / dist_bh) * f_bh + tangent_x * swirl
                fy = (dy_bh / dist_bh) * f_bh + tangent_y * swirl
            else:
                fx, fy = 0, 0

            # Inter-particle Clumping
            if p.particle_type in ("micro", "standard"):
                for m in macro_particles:
                    if m is not p:
                        cdx = m.x - p.x
                        cdy = m.y - p.y
                        cdist = math.sqrt(cdx**2 + cdy**2)

                        if 3.0 < cdist < m.radius * 4.8:
                            f_c = (m.mass * 6.0) / (cdist**2 + 20.0)
                            fx += (cdx / cdist) * f_c
                            fy += (cdy / cdist) * f_c

                            if cdist < m.radius * 1.4:
                                f_repel = 12.0 / (cdist + 1.2)
                                fx -= (cdx / cdist) * f_repel
                                fy -= (cdy / cdist) * f_repel

            fx += 0.9

            p.vx += (fx / p.mass) * dt
            p.vy += (fy / p.mass) * dt

            v_mag = math.sqrt(p.vx**2 + p.vy**2)
            if v_mag > 12.5:
                p.vx = (p.vx / v_mag) * 12.5
                p.vy = (p.vy / v_mag) * 12.5
                v_mag = 12.5

            p.x += p.vx * dt
            p.y += p.vy * dt

            if step % 3 == 0:
                p.history.append((p.x, p.y, v_mag))

    return particles, (bh_x, bh_y, r_event, r_photon)


def hsl_to_hex(h, s, l):
    # Convert HSL (h in [0,360], s and l in [0,1]) to RGB hex string
    r, g, b = colorsys.hls_to_rgb(h / 360.0, l, s)
    return f"#{int(r*255):02x}{int(g*255):02x}{int(b*255):02x}"


def get_gradient_mixed_color(x, y, v, color_seed, bh_x, bh_y):
    # CONTINUOUS HSL GRADIENT & ORGANIC TURBULENCE COLOR MIXING
    t = min(1.0, max(0.0, (x + 100) / 1150.0))
    
    # Organic spatial noise for smooth particle-to-particle color blending
    noise = math.sin(x * 0.012 + y * 0.018) * 0.08 + color_seed
    t_mixed = min(1.0, max(0.0, t + noise))

    dist_bh = math.sqrt((x - bh_x)**2 + (y - bh_y)**2)

    # Smooth Continuous Gradient HSL Sweep:
    # 0.0 -> 0.35: Deep Ocean Cyan (195°) to Electric Violet (275°)
    # 0.35 -> 0.60: Electric Violet (275°) to Deep Magenta (320°)
    # 0.60 -> 0.85: Deep Magenta (320°) to Relativistic Gold (38°)
    # 0.85 -> 1.00: Relativistic Gold (38°) to Fiery Copper (18°)

    if t_mixed < 0.35:
        u = t_mixed / 0.35
        hue = 195 + u * 80
        sat = 0.92
        lum = 0.48
    elif t_mixed < 0.60:
        u = (t_mixed - 0.35) / 0.25
        hue = 275 + u * 45
        sat = 0.88
        lum = 0.47
    elif t_mixed < 0.85:
        u = (t_mixed - 0.60) / 0.25
        hue = (320 + u * 78) % 360
        sat = 0.94
        lum = 0.50
    else:
        u = (t_mixed - 0.85) / 0.15
        hue = 38 - u * 20
        sat = 0.96
        lum = 0.48

    # High energy excitation near black hole or high velocity shifts tone slightly warmer/brighter
    if dist_bh < 160 or v > 8.0:
        lum = min(0.68, lum + 0.12)

    head_hex = hsl_to_hex(hue, sat, lum)
    trail_hex = hsl_to_hex(hue, max(0.55, sat - 0.15), max(0.16, lum - 0.26))

    return head_hex, trail_hex


def render_td_monogram(cx, cy, scale=1.0):
    stroke_w = 4.5 * scale
    out = []

    out.append(f'''
    <!-- Relativistic "TD" Monogram Symbol -->
    <g opacity="0.98" filter="url(#subtleGlow)">
        <!-- Black Drop Shadow for TD Monogram -->
        <g stroke="#000000" stroke-width="{stroke_w + 3.0:.1f}" fill="none" stroke-linecap="round" stroke-linejoin="round" opacity="0.9">
            <path d="M {cx-28*scale:.1f},{cy-20*scale:.1f} L {cx+12*scale:.1f},{cy-20*scale:.1f}"/>
            <path d="M {cx-10*scale:.1f},{cy-20*scale:.1f} L {cx-10*scale:.1f},{cy+22*scale:.1f}"/>
            <path d="M {cx-10*scale:.1f},{cy-20*scale:.1f} C {cx+28*scale:.1f},{cy-20*scale:.1f} {cx+34*scale:.1f},{cy+22*scale:.1f} {cx-10*scale:.1f},{cy+22*scale:.1f}"/>
        </g>

        <!-- Foreground Glowing "TD" Monogram -->
        <path d="M {cx-28*scale:.1f},{cy-20*scale:.1f} L {cx+12*scale:.1f},{cy-20*scale:.1f}" stroke="#f59e0b" stroke-width="{stroke_w:.1f}" stroke-linecap="round" fill="none"/>
        <path d="M {cx-10*scale:.1f},{cy-20*scale:.1f} L {cx-10*scale:.1f},{cy+22*scale:.1f}" stroke="#ea580c" stroke-width="{stroke_w:.1f}" stroke-linecap="round" fill="none"/>
        <path d="M {cx-10*scale:.1f},{cy-20*scale:.1f} C {cx+28*scale:.1f},{cy-20*scale:.1f} {cx+34*scale:.1f},{cy+22*scale:.1f} {cx-10*scale:.1f},{cy+22*scale:.1f}" stroke="#f59e0b" stroke-width="{stroke_w:.1f}" stroke-linecap="round" fill="none"/>

        <!-- Inner Focus Point -->
        <circle cx="{cx+8*scale:.1f}" cy="{cy+1*scale:.1f}" r="{3.5*scale:.1f}" fill="#ffffff"/>
        <circle cx="{cx+8*scale:.1f}" cy="{cy+1*scale:.1f}" r="{6.0*scale:.1f}" fill="none" stroke="#ea580c" stroke-width="1.5"/>
    </g>
    ''')
    return "\n".join(out)


def render_velocity_dependent_gradient_trails(particles, bh_x, bh_y, width):
    svg_parts = []
    svg_parts.append('<!-- GRADIENT MIXED VELOCITY TRAILS: Low velocity = Brighter/Thicker, High velocity = Faint/Blurred -->')
    svg_parts.append('<g filter="url(#subtleGlow)">')

    for p in particles:
        if p.particle_type in ("macro", "medium") or (p.particle_type == "standard" and random.random() > 0.35):
            if len(p.history) >= 2:
                base_w = max(0.9, p.radius * 0.55)

                for k in range(len(p.history) - 1):
                    x1, y1, v1 = p.history[k]
                    x2, y2, v2 = p.history[k+1]

                    if -60 <= x1 <= width + 60 and -60 <= x2 <= width + 60:
                        v_avg = (v1 + v2) * 0.5

                        opacity = max(0.18, min(0.88, 0.90 - (v_avg / 12.0) * 0.68))
                        stroke_w = max(0.6, base_w * (1.3 - (v_avg / 12.0) * 0.65))

                        head_c, trail_c = get_gradient_mixed_color(x1, y1, v1, p.color_seed, bh_x, bh_y)
                        seg_col = head_c if v_avg < 4.5 else trail_c

                        svg_parts.append(f'<line x1="{x1:.1f}" y1="{y1:.1f}" x2="{x2:.1f}" y2="{y2:.1f}" stroke="{seg_col}" stroke-width="{stroke_w:.1f}" stroke-opacity="{opacity:.2f}" stroke-linecap="round"/>')

    svg_parts.append('</g>')
    return "\n".join(svg_parts)


def create_time_dilation_logo_v10():
    width = 1000
    height = 600
    particles, (bh_x, bh_y, r_event, r_photon) = run_pure_particle_simulation(num_steps=150, width=width, height=height)

    svg_parts = []

    # SVG Header - DEEP WARM OBSIDIAN WITH TRANSPARENT ALPHA BACKGROUND
    svg_parts.append(f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" width="100%" height="100%">
    <defs>
        <radialGradient id="bgGlow" cx="44%" cy="50%" r="65%">
            <stop offset="0%" stop-color="#1a061e" stop-opacity="0.35"/>
            <stop offset="45%" stop-color="#090310" stop-opacity="0.10"/>
            <stop offset="100%" stop-color="#040207" stop-opacity="0.0"/>
        </radialGradient>

        <radialGradient id="singularityGlow" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stop-color="#000000"/>
            <stop offset="50%" stop-color="#050109"/>
            <stop offset="78%" stop-color="#c026d3" stop-opacity="0.75"/>
            <stop offset="90%" stop-color="#ea580c" stop-opacity="0.85"/>
            <stop offset="98%" stop-color="#f59e0b" stop-opacity="0.9"/>
            <stop offset="100%" stop-color="#f59e0b" stop-opacity="0"/>
        </radialGradient>

        <linearGradient id="dopplerLightGrad" x1="0%" y1="0%" x2="100%" y2="100%">
            <stop offset="0%" stop-color="#0284c7" stop-opacity="0.18"/>
            <stop offset="35%" stop-color="#9333ea" stop-opacity="0.15"/>
            <stop offset="70%" stop-color="#ea580c" stop-opacity="0.12"/>
            <stop offset="100%" stop-color="#f59e0b" stop-opacity="0.04"/>
        </linearGradient>

        <!-- Filters -->
        <filter id="subtleGlow" x="-30%" y="-30%" width="160%" height="160%">
            <feGaussianBlur stdDeviation="3" result="blur"/>
            <feMerge>
                <feMergeNode in="blur"/>
                <feMergeNode in="SourceGraphic"/>
            </feMerge>
        </filter>

        <filter id="blackTextShadow" x="-20%" y="-20%" width="140%" height="140%">
            <feDropShadow dx="3" dy="4" stdDeviation="3" flood-color="#000000" flood-opacity="1.0"/>
            <feDropShadow dx="1" dy="2" stdDeviation="1" flood-color="#000000" flood-opacity="0.9"/>
        </filter>
    </defs>

    <rect width="{width}" height="{height}" fill="url(#bgGlow)"/>

    <!-- 1. Gravitational Light Lensing Arc -->
    <g opacity="0.45" style="mix-blend-mode: screen;">
        <path d="M -100,-50 C 200,-20 {bh_x-120},{bh_y-r_photon-50} {bh_x},{bh_y-r_photon-35} C {bh_x+120},{bh_y-r_photon-20} 800,200 1100,500 L 1050,550 C 750,250 {bh_x+150},{bh_y-r_photon} {bh_x},{bh_y-r_photon-15} C {bh_x-150},{bh_y-r_photon-30} 150,-10 -150,-10 Z" fill="url(#dopplerLightGrad)"/>
    </g>

    <!-- 2. Warm Spacetime Grid Lines -->
    <g stroke="#9333ea" stroke-opacity="0.12" stroke-width="0.9" fill="none">
    ''')

    for r in range(100, 540, 42):
        svg_parts.append(f'<ellipse cx="{bh_x}" cy="{bh_y}" rx="{r*1.28}" ry="{r*0.68}" transform="rotate(-15 {bh_x} {bh_y})"/>')

    svg_parts.append('</g>')

    # 3. Particle Physics Accretion Rings
    svg_parts.append('<!-- Particle Physics Accretion Rings -->')
    svg_parts.append(f'''
    <g filter="url(#subtleGlow)">
        <ellipse cx="{bh_x}" cy="{bh_y}" rx="74" ry="168" fill="none" stroke="#c026d3" stroke-width="4.5" opacity="0.45" transform="rotate(8 {bh_x} {bh_y})"/>
        <ellipse cx="{bh_x}" cy="{bh_y}" rx="265" ry="74" fill="none" stroke="#7e22ce" stroke-width="6" opacity="0.35" transform="rotate(-16 {bh_x} {bh_y})"/>
        <ellipse cx="{bh_x}" cy="{bh_y}" rx="205" ry="54" fill="none" stroke="#ea580c" stroke-width="8" opacity="0.6" transform="rotate(-16 {bh_x} {bh_y})"/>
        <ellipse cx="{bh_x}" cy="{bh_y}" rx="145" ry="35" fill="none" stroke="#f59e0b" stroke-width="6" opacity="0.75" transform="rotate(-16 {bh_x} {bh_y})"/>
    </g>
    ''')

    # 4. CONTINUOUS GRADIENT MIXED MOVEMENT TRAILS
    svg_parts.append(render_velocity_dependent_gradient_trails(particles, bh_x, bh_y, width))

    # 5. PURE PARTICLES WITH CONTINUOUS GRADIENT & TURBULENT COLOR MIXING
    svg_parts.append('<!-- Pure Physics Particles with Continuous HSL Gradient Mixing -->')

    for p_type in ("micro", "standard", "medium", "macro"):
        svg_parts.append(f'<!-- Layer: {p_type} particles -->')
        for p in particles:
            if p.particle_type == p_type and -30 <= p.x <= width + 40 and -30 <= p.y <= height + 40:
                head_col, _ = get_gradient_mixed_color(p.x, p.y, math.sqrt(p.vx**2 + p.vy**2), p.color_seed, bh_x, bh_y)

                if p_type == "micro":
                    svg_parts.append(f'<circle cx="{p.x:.1f}" cy="{p.y:.1f}" r="{p.radius:.1f}" fill="{head_col}" opacity="{random.uniform(0.55, 0.88):.2f}"/>')
                elif p_type == "standard":
                    svg_parts.append(f'<circle cx="{p.x:.1f}" cy="{p.y:.1f}" r="{p.radius:.1f}" fill="{head_col}" opacity="{random.uniform(0.8, 0.98):.2f}"/>')
                elif p_type == "medium":
                    svg_parts.append(f'<circle cx="{p.x:.1f}" cy="{p.y:.1f}" r="{p.radius:.1f}" fill="{head_col}" stroke="#ffffff" stroke-width="0.8" stroke-opacity="0.4" opacity="0.95"/>')
                elif p_type == "macro":
                    svg_parts.append(f'''
                    <g filter="url(#subtleGlow)">
                        <circle cx="{p.x:.1f}" cy="{p.y:.1f}" r="{p.radius*1.35:.1f}" fill="{head_col}" opacity="0.35"/>
                        <circle cx="{p.x:.1f}" cy="{p.y:.1f}" r="{p.radius:.1f}" fill="{head_col}" stroke="#ffffff" stroke-width="1.2" opacity="0.98"/>
                        <circle cx="{p.x - p.radius*0.2:.1f}" cy="{p.y - p.radius*0.3:.1f}" r="{p.radius*0.35:.1f}" fill="#ffffff" opacity="0.8"/>
                    </g>
                    ''')

    # 6. Black Hole Core & RELATIVISTIC "TD" MONOGRAM LOGO SYMBOL
    svg_parts.append('<!-- Black Hole Core & Relativistic TD Monogram Logo Symbol -->')
    svg_parts.append(f'<circle cx="{bh_x}" cy="{bh_y}" r="{r_photon * 1.85}" fill="url(#singularityGlow)" filter="url(#subtleGlow)"/>')

    svg_parts.append(f'''
    <g filter="url(#subtleGlow)">
        <circle cx="{bh_x}" cy="{bh_y}" r="{r_photon}" fill="none" stroke="#ea580c" stroke-width="3.5" opacity="0.85"/>
        <circle cx="{bh_x}" cy="{bh_y}" r="{r_photon - 2.5}" fill="none" stroke="#f59e0b" stroke-width="1.8" opacity="0.9"/>
    </g>
    ''')

    # Pitch Black Hole Core
    svg_parts.append(f'<circle cx="{bh_x}" cy="{bh_y}" r="{r_event}" fill="#020105" stroke="#ea580c" stroke-width="1.2" opacity="1.0"/>')

    # Render "TD" Monogram Logo in Center of Black Hole!
    svg_parts.append(render_td_monogram(bh_x, bh_y, scale=1.0))

    # 7. Integrated Typography with Solid Black Drop Shadows
    svg_parts.append('<!-- Integrated Typography with Solid Black Drop Shadows -->')
    svg_parts.append('''
    <g transform="translate(60, 535)" filter="url(#blackTextShadow)">
        <text x="3" y="3" font-family="'Inter', 'Outfit', 'Roboto', sans-serif" font-size="26" font-weight="900" letter-spacing="6" fill="#000000" opacity="1.0">
            TIME DILATION DAW
        </text>
        <text x="5" y="25" font-family="'Inter', 'Outfit', 'Roboto', sans-serif" font-size="11" font-weight="600" letter-spacing="9" fill="#000000" opacity="1.0">
            RELATIVISTIC AUDIO ENGINE
        </text>

        <text x="0" y="0" font-family="'Inter', 'Outfit', 'Roboto', sans-serif" font-size="26" font-weight="900" letter-spacing="6" fill="#ffffff">
            TIME DILATION <tspan fill="#f59e0b">DAW</tspan>
        </text>
        <text x="2" y="22" font-family="'Inter', 'Outfit', 'Roboto', sans-serif" font-size="11" font-weight="600" letter-spacing="9" fill="#ea580c" opacity="0.95">
            RELATIVISTIC AUDIO ENGINE
        </text>
    </g>
    ''')

    svg_parts.append('</svg>')
    return "\n".join(svg_parts)


def create_icon_only_logo_v10():
    width = 512
    height = 512
    particles, (bh_x, bh_y, r_event, r_photon) = run_pure_particle_simulation(num_steps=150, width=width, height=height)

    bh_x_icon, bh_y_icon = 256, 256
    r_event_icon, r_photon_icon = 72, 94

    svg_parts = []
    svg_parts.append(f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" width="100%" height="100%">
    <defs>
        <radialGradient id="bgGlowIcon" cx="50%" cy="50%" r="75%">
            <stop offset="0%" stop-color="#1a061e" stop-opacity="0.35"/>
            <stop offset="50%" stop-color="#090310" stop-opacity="0.10"/>
            <stop offset="100%" stop-color="#040207" stop-opacity="0.0"/>
        </radialGradient>

        <radialGradient id="singularityGlowIcon" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stop-color="#000000"/>
            <stop offset="50%" stop-color="#050109"/>
            <stop offset="78%" stop-color="#c026d3" stop-opacity="0.8"/>
            <stop offset="90%" stop-color="#ea580c" stop-opacity="0.9"/>
            <stop offset="98%" stop-color="#f59e0b" stop-opacity="0.95"/>
            <stop offset="100%" stop-color="#f59e0b" stop-opacity="0"/>
        </radialGradient>

        <filter id="subtleGlowIcon" x="-30%" y="-30%" width="160%" height="160%">
            <feGaussianBlur stdDeviation="3" result="blur"/>
            <feMerge>
                <feMergeNode in="blur"/>
                <feMergeNode in="SourceGraphic"/>
            </feMerge>
        </filter>
    </defs>

    <rect width="{width}" height="{height}" rx="100" fill="url(#bgGlowIcon)"/>

    <!-- Light Rays -->
    <g opacity="0.45" style="mix-blend-mode: screen;">
        <path d="M -50,-20 C 150,-10 {bh_x_icon-60},{bh_y_icon-r_photon_icon-30} {bh_x_icon},{bh_y_icon-r_photon_icon-20} C {bh_x_icon+60},{bh_y_icon-r_photon_icon-10} 400,100 550,300 L 520,350 C 350,150 {bh_x_icon+80},{bh_y_icon-r_photon_icon} {bh_x_icon},{bh_y_icon-r_photon_icon-5} C {bh_x_icon-80},{bh_y_icon-r_photon_icon-10} 80,0 -80,10 Z" fill="#ea580c" opacity="0.25"/>
    </g>

    <!-- Einstein Lensing Arc -->
    <g filter="url(#subtleGlowIcon)">
        <ellipse cx="{bh_x_icon}" cy="{bh_y_icon}" rx="76" ry="172" fill="none" stroke="#c026d3" stroke-width="5" opacity="0.5" transform="rotate(8 {bh_x_icon} {bh_y_icon})"/>
    </g>

    <!-- Gradient Velocity Trails in Icon -->
    <g filter="url(#subtleGlowIcon)">
    ''')

    for p in particles[::4]:
        if len(p.history) >= 2:
            base_w = max(0.9, p.radius * 0.5)
            for k in range(len(p.history) - 1):
                x1, y1, v1 = p.history[k]
                x2, y2, v2 = p.history[k+1]
                ix1, iy1 = x1 * 0.5 + 40, y1 * 0.8 + 50
                ix2, iy2 = x2 * 0.5 + 40, y2 * 0.8 + 50

                if -30 <= ix1 <= width + 30 and -30 <= ix2 <= width + 30:
                    v_avg = (v1 + v2) * 0.5
                    opacity = max(0.18, min(0.88, 0.90 - (v_avg / 12.0) * 0.68))
                    stroke_w = max(0.6, base_w * (1.3 - (v_avg / 12.0) * 0.65))

                    head_c, trail_c = get_gradient_mixed_color(x1, y1, v1, p.color_seed, bh_x, bh_y)
                    seg_col = head_c if v_avg < 4.5 else trail_c
                    svg_parts.append(f'<line x1="{ix1:.1f}" y1="{iy1:.1f}" x2="{ix2:.1f}" y2="{iy2:.1f}" stroke="{seg_col}" stroke-width="{stroke_w:.1f}" stroke-opacity="{opacity:.2f}" stroke-linecap="round"/>')

    svg_parts.append('</g>')

    # Pure Particles in Icon
    svg_parts.append('<g filter="url(#subtleGlowIcon)">')
    for p in particles[::2]:
        px_i = p.x * 0.5 + 40
        py_i = p.y * 0.8 + 50
        if 0 <= px_i <= width and 0 <= py_i <= height:
            head_col, _ = get_gradient_mixed_color(p.x, p.y, math.sqrt(p.vx**2 + p.vy**2), p.color_seed, bh_x, bh_y)
            svg_parts.append(f'<circle cx="{px_i:.1f}" cy="{py_i:.1f}" r="{p.radius*0.85:.1f}" fill="{head_col}" opacity="{random.uniform(0.75, 0.98):.2f}"/>')
    svg_parts.append('</g>')

    # Black Hole Core
    svg_parts.append(f'<circle cx="{bh_x_icon}" cy="{bh_y_icon}" r="{r_photon_icon * 1.85}" fill="url(#singularityGlowIcon)"/>')
    svg_parts.append(f'<circle cx="{bh_x_icon}" cy="{bh_y_icon}" r="{r_photon_icon}" fill="none" stroke="#ea580c" stroke-width="3.2" opacity="0.85"/>')
    svg_parts.append(f'<circle cx="{bh_x_icon}" cy="{bh_y_icon}" r="{r_event_icon}" fill="#020105" stroke="#ea580c" stroke-width="1.2"/>')

    # Render "TD" Monogram in Center of App Icon!
    svg_parts.append(render_td_monogram(bh_x_icon, bh_y_icon, scale=1.0))

    svg_parts.append('</svg>')
    return "\n".join(svg_parts)


if __name__ == "__main__":
    os.makedirs("Source/assets", exist_ok=True)
    os.makedirs("web", exist_ok=True)
    os.makedirs("scripts", exist_ok=True)

    banner_svg = create_time_dilation_logo_v10()
    icon_svg = create_icon_only_logo_v10()

    with open("Source/assets/logo_banner.svg", "w") as f:
        f.write(banner_svg)

    with open("Source/assets/logo_icon.svg", "w") as f:
        f.write(icon_svg)

    with open("web/logo_banner.svg", "w") as f:
        f.write(banner_svg)

    with open("web/logo_icon.svg", "w") as f:
        f.write(icon_svg)

    print("v10 Relativistic Continuous HSL Gradient Mixed Particle Logo generated successfully in Source/assets/ and web/")
