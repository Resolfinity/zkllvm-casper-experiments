This folder consists of ts files with experiments of whether validators root update is possible without fully re-merkelization.

E.g. given old validators set merkle tree, list of updated and added validators, it is possible to calculate updated validators set root, using multi proofs of changed leaves.

Conclusion: it looks achievable to get new validators root using only updated leaves and their multi-merkle-proofs.
