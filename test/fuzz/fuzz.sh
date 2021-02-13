test=parser

corpus="./test/fuzz/${test}-corpus"

if [ ! -e "$corpus" ];
then mkdir -p "$corpus"
fi

clang test/fuzz/fuzz_$test.c -I src/ -O2 -g -fsanitize=fuzzer,address,signed-integer-overflow -o fuzz
./fuzz "$corpus" -timeout=1 "$@" -only_ascii=1 -dict=test/fuzz/token_dict.txt && rm ./fuzz
