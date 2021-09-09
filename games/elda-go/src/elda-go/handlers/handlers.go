package handlers

import (
	"reflect"

	"elda-go/handlers/filein"
	"elda-go/handlers/pipein"
	"elda-go/handlers/stdin"
	"elda-go/handlers/ticker"

	"elda-go/handlers/fileout"
	"elda-go/handlers/pipeout"
	"elda-go/handlers/stdout"
	"elda-go/handlers/xdo"

	"elda-go/handlers/edsm"
	"elda-go/handlers/igau"
)

type Handler interface {
	Name() string
	Init(map[string]string) error
	Type() int
	Push(string) error
	Pull() (string, error)
	Done()
}

var registeredHandlers []Handler

func init() {
	registeredHandlers = []Handler{
		filein.Register(),
		pipein.Register(),
		stdin.Register(),
		ticker.Register(),

		fileout.Register(),
		pipeout.Register(),
		stdout.Register(),
		xdo.Register(),

		edsm.Register(),
		igau.Register(),
	}
}

func Search(name string, htype int) Handler {
	for _, h := range registeredHandlers {
		if h.Name() == name && (h.Type()&htype) > 0 {
			return h
		}
	}
	return nil
}

// heavy magic here:
// we want to make new handler variable but can not do this directly
// because its package name is not known to a calling func
// thus we use this dirty trick extracting its definition from an interface
// and making a copy of it zeroing all its internals
func DupAndZero(i interface{}) Handler {
	return reflect.New(reflect.ValueOf(i).Elem().Type()).Interface().(Handler)
}
