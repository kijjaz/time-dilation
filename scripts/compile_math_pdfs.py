import os
import glob
import subprocess

def compile_all_tex_files():
    math_dir = os.path.abspath("math")
    tex_files = sorted(glob.glob(os.path.join(math_dir, "*.tex")))
    
    print(f"Found {len(tex_files)} LaTeX (.tex) papers in {math_dir}:")
    
    compiled_count = 0
    for tex_path in tex_files:
        basename = os.path.basename(tex_path)
        pdf_path = tex_path.replace(".tex", ".pdf")
        print(f"Compiling {basename} using Tectonic TeX Engine...")
        
        try:
            res = subprocess.run(
                ["/opt/homebrew/bin/tectonic", tex_path],
                capture_output=True,
                text=True,
                check=True
            )
            print(f"  -> Generated {os.path.basename(pdf_path)} ({os.path.getsize(pdf_path)} bytes)")
            compiled_count += 1
        except subprocess.CalledProcessError as e:
            print(f"  ERROR compiling {basename}: {e.stderr}")

    print(f"\nSuccessfully compiled {compiled_count}/{len(tex_files)} LaTeX papers to vector PDFs!")

if __name__ == "__main__":
    compile_all_tex_files()
