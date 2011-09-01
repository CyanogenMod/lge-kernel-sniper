struct proxi_switch_platform_data {
	const char	*name;
	unsigned 	gpio;

	/* if NULL, switch_dev.name will be printed */
	const char	*name_on;
	const char	 *name_off;
	/* if NULL, "0" or "1" will be printed */
	const char	*state_on;
	const char	*state_off;
};

