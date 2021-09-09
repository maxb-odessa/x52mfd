package edsm

import (
	"time"

	"elda-go/def"
)

var buf string

// any name
type handler struct {
	// mandatory fields
	name string
	typ  int

	// optional
	buf string
}

// register us
func Register() *handler {
	return &handler{
		name: "edsm",
		typ:  def.HANDLER_TYPE_ACTION | def.HANDLER_TYPE_SOURCE,
	}
}

func (self *handler) Init(vars map[string]string) error {
	return nil
}

func (self *handler) Name() string {
	return self.name
}

func (self *handler) Type() int {
	return self.typ
}

func (self *handler) Push(s string) error {
	buf = s
	return nil
}

func (self *handler) Pull() (string, error) {
	time.Sleep(time.Second * 3)
	return buf, nil
}

func (self *handler) Done() {
}
