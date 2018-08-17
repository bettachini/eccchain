#!/usr/bin/octave -fq
pkg ("load", "auto");

# Format matrix to boost::ublas style
function printMatrix(M)
	printf("[%d,%d]\n",size(M)(1),size(M)(2))
	printf "("
	for y=1:size(M)(1)
		printf "("
		for x=1:size(M)(2)
			printf("%d,",M(y,x))
		end
		if (y < size(M)(1))
			printf "),\n"
		else	printf ")\n"
		endif
	end
	printf ")\n"
endfunction

arg_list = argv ()
if (nargin==1)
	[h,g] = hammgen(str2num(arg_list{1}))
	printMatrix(h)
	printMatrix(g)
else	printf("Usage: hammingGenMatrix <M> \n")
endif
