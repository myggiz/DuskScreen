# Static analysis

`.clang-tidy` in the repository root configures clang-tidy. The check list is
deliberately short — see the comment at the top of that file for why.

## Running it

clang-tidy needs a compilation database, and qmake does not produce one. Build
first, then generate `compile_commands.json` from the generated Makefile:

```bash
mkdir -p build && cd build
qmake6 ../duskscreen.pro && make -j$(nproc)

python3 - <<'EOF'
import json, os, re
mk = open('Makefile').read()
inc = re.search(r'^INCPATH\s*=\s*(.*)$', mk, re.M).group(1)
dfs = re.search(r'^DEFINES\s*=\s*(.*)$', mk, re.M).group(1)
root = os.path.abspath('..')
srcs = sorted(os.path.join(dp, f)
              for dp, _, fn in os.walk(root) if '.git' not in dp
              for f in fn if f.endswith('.cpp'))
json.dump([{"directory": os.getcwd(), "file": s,
            "command": f"g++ -std=c++17 -fPIC {dfs} {inc} -c {s}"}
           for s in srcs], open('compile_commands.json', 'w'), indent=1)
EOF

cd .. && clang-tidy -p build $(git ls-files '*.cpp' | grep -v SingleApplication)
```

The working tree is expected to come back clean.

## What it cannot see

clang-tidy only analyses the branches the preprocessor keeps, so running it on
Linux skips every `#if defined(Q_OS_WIN)` block — a large part of the capture
and window-picker code. Findings in Windows-only code have to come from a run on
Windows, or from the compiler's own warnings there.

`tools/SingleApplication/` is excluded. It is vendored upstream code, updated by
replacing the directory rather than by patching it, so findings in it are noise.
