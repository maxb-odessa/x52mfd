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
	inChan  chan string
}

func New() *Action {
	act := new(Action)
	act.vars = make(map[string]string)
	act.inChan = make(chan string, def.ACT_CHAN_LEN)
	return act
}

func (self *Action) Name() string {
	return self.name
}

func (self *Action) SetName(name string) error {
	if self.name != "" {
		return fmt.Errorf("can not set action name to '%s': already set to '%s'", name, self.name)
	}
	self.name = name
	return nil
}

func (self *Action) SetVar(key, val string) error {
	if v, ok := self.vars[key]; ok {
		return fmt.Errorf("variable '%s' already set to '%s'", key, v)
	}
	self.vars[key] = val
	return nil
}

func (self *Action) SetHandler(name string) error {
	if self.handler != nil {
		return fmt.Errorf("handler already defined")
	}
	self.handler = handlers.Search(name, def.HANDLER_TYPE_ACTION)
	return nil
}

func (self *Action) Run() {
	// go(): wait for data from self.inChan and call a handler
}
