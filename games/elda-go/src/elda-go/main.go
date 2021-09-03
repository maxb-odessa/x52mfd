package main

import (
	"fmt"
	"os"
	"sync"

	"elda-go/config"
	"elda-go/def"
	"elda-go/events"
	"elda-go/log"
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
	cfg, err := config.New(confFile)
	if err != nil {
		log.Err("Config file '%s' error: %v\n", confFile, err)
		os.Exit(2)
	} else if err = cfg.Parse(); err != nil {
		log.Err("Failed to parse config file '%s': %v\n", confFile, err)
		os.Exit(3)
	}

	var wg sync.WaitGroup

	// start actions
	for name, act := range cfg.Actions() {
		if err := act.Init(); err != nil {
			log.Err("failed to init action '%s': %v\n", name, err)
			log.Err("action '%s' is disabled\n", name)
		} else {
			log.Info("starting action '%s'\n", name)
			wg.Add(1)
			a := act
			go func() { defer wg.Done(); a.Run() }()
		}
	}

	// start sources
	for name, src := range cfg.Sources() {
		if err := src.Init(); err != nil {
			log.Err("failed to init source '%s': %v\n", name, err)
			log.Err("source '%s' is disabled\n", name)
		} else {
			log.Info("starting source '%s'\n", name)
			wg.Add(1)
			s := src
			go func() { defer wg.Done(); s.Run() }()
		}
	}

	// start events
	events.Run(cfg.Sources(), cfg.Actions(), cfg.Events())

	wg.Wait()

	log.Info("exiting\n")
	return
}
