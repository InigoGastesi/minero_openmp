ehp:
	gcc src/sgssi_miner.c src/sha256calc.c -o minero -lssl -lcrypto -lm -fopenmp