GROMACS
=======

This is an unofficial Gromacs version with additional kernels using a distance dependent dielectric constant.
This modification is primarily intended for use with the EEF1-SB implicit solvent force field implemented in Plumed. The implicit solvent is optimized for use with CHARMM36, which is bundled with this Gromacs version.

Usage
-----

Use this just like regular Gromacs, but specify `userint1 = 1` to activate the distance dependent dielectric constant kernels and set `epsilon-r` to the desired factor. This modification is currently only implemented for SIMD and not for GPU kernels.
