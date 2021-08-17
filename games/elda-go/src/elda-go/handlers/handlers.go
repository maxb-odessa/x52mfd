package handlers

import (
	_ "elda-go/handlers/filein"
	_ "elda-go/handlers/interval"
	_ "elda-go/handlers/pipein"
	_ "elda-go/handlers/stdin"

	_ "elda-go/handlers/fileout"
	_ "elda-go/handlers/pipeout"
	_ "elda-go/handlers/stdout"
	_ "elda-go/handlers/xdo"
)

const (
	TYPE_SOURCE = iota
	TYPE_ACTION
)

type Handler interface {
	Init(map[string]string) error
	Type() int
	Name() string
	Exec(string) error
}

var LoadedHandlers []Handler

func Search(name string, htype int) Handler {

	for _, h := range LoadedHandlers {
		if h.Name() == name && h.Type() == htype {
			return h
		}
	}

	return nil
}
