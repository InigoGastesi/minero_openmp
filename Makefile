ehp:
	gcc sgssi_miner.c sha256calc.c -o minero -lssl -lcrypto -lm -fopenmp