we have prebuilt merkle tree of 2\*\*21 validators for internal use

each validator has following 5 fields

pubkey
activation_epoch
exit_epoch
balance
index

for each epoch, we have up to 25 validators changed.

we want to represent them updating poseidon merkle tree

to update merkle tree of 21 heigth, we have to:

- hash validators leaves , which is 5 hashes each
- make up to 22 recalculations of upper nodes

for total 27 hashes \* 25 validators, to update poseidon tree root, we have to make 675 hashes approx.
