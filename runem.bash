for falloff in 0.1 0.3 0.5 0.7 0.9 1.0
do
   for sigma in 1 3 5 7 9 11
   do
      ./gabble --SeedDensityFalloff=$falloff --FinalSnapshot=0 --SigmaOfGaussian=$sigma
   done
done
