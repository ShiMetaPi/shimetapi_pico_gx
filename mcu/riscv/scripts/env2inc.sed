

/^[A-Za-z][A-Za-z0-9_]*/ {
	# add #define to the line head
	s/^/#define /;
	# remove '='
	s/=/ /;
	p
}

