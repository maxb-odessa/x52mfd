package action

import (
	"fmt"

	"elda-go/def"
	"elda-go/handlers"
)

type Action struct {
	name    string
	vars    map[string]string
	handler handlers.Handler
	ch      chan string
}

func New() *Action {
	a := new(Action)
	a.vars = make(map[string]string)
	a.ch = make(chan string, def.SRC_CHAN_LEN)
	return a
}

func (self *Action) Name() string {
	return self.name
}

func (self *Action) SetName(name string) error {
	self.name = name
	return nil
}

func (self *Action) SetVar(key, val string) (err error) {
	if _, ok := self.vars[key]; !ok {
		self.vars[key] = val
	} else {
		err = fmt.Errorf("variable '%s' already set to '%s'", key, val)
	}
	return
}

func (self *Action) SetHandler(name string) error {

	self.handler = handlers.Search(name, handlers.TYPE_ACTION)

	return nil
}
