the purpose of these experiments is to determine how many multi-proofs of changed validators could be proved and for how long.

we suppose that each epoch, there will be at least 14 validators changed.

max count of validators, changed during the epoch is still subject to research.

in case of skipped epochs, changed validators count increased.

from some random research, for 10 hours (60 epochs) there was found 1500 validators changed, hence we have 25 validators changed during one epoch, approx.

we are operating 2\*\*21 validators list, hence merkle multi-proof has length 450 approx.

to verify multi-proof, we have to merkelize validator, it takes 15 hashes

summary: we can have measured 1 epoch validator set changes multiproof with 450+15\*25 = 825 sha256 hashes.
