/* rewinds mag tape drive */
main(argc,argv) char**argv; {
	char *f;
	int fd;

	if (argc > 1) f = argv[1];
		else  f = "/dev/mt0";

	fd = open(f,0);
	if (fd < 0) printf("Can't open %s\n",f);
		else close(fd);
}
