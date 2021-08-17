package main

import (
	"fmt"
	"os"

	"elda-go/config"
	"elda-go/def"
	_ "elda-go/event"
	"elda-go/log"

	// load handlers
	_ "elda-go/handlers"
)

// TODO show some help
func showHelp(progname string) {
	fmt.Fprintf(os.Stderr,
		"Usage:\n%s [config file]\n"+
			"Default config file is '%s'\n",
		progname,
		def.ConfFile)
}

// the main part
func main() {

	var confFile string

	// get config file path
	switch len(os.Args) {
	case 1:
		confFile = def.ConfFile
	case 2:
		confFile = os.Args[1]
	default:
		showHelp(os.Args[0])
		os.Exit(1)
	}

	// read config
	if cfg, err := config.New(confFile); err != nil {
		log.Err("Config file '%s' error: %v\n", confFile, err)
		os.Exit(2)
	} else if err = cfg.Parse(); err != nil {
		log.Err("Failed to read config file '%s': %v\n", confFile, err)
		os.Exit(3)
	}

	// run!
	//events.Run()

	return
}
