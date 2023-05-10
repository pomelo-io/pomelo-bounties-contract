SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
BASE_DIR=$(dirname "$(dirname "$SCRIPT_DIR")")

docker run --rm -v ${BASE_DIR}:/app zzptichka/antelope-cdt-3.1 sh -c "cd pomelo-bounties-contract && cdt-cpp work.pomelo.cpp -I include"
