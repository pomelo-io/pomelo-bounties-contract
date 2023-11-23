SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
BASE_DIR="/host$(dirname "$SCRIPT_DIR")"

dune -- cdt-cpp ${BASE_DIR}/work.pomelo.cpp -I ${BASE_DIR}/include --no-missing-ricardian-clause -o=${BASE_DIR}/work.pomelo.wasm
