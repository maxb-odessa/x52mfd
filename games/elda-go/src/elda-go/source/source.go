package source

import (
	"fmt"

	"elda-go/def"
	"elda-go/handlers"
)

type Source struct {
	name    string
	vars    map[string]string
	handler handlers.Handler
	outChan chan def.ChanMsg // TODO
}

func New() *Source {
	s := new(Source)
	s.vars = make(map[string]string)
	s.ch = make(chan string, def.ACT_CHAN_LEN)
	return s
}

func (self *Source) Name() string {
	return self.name
}

func (self *Source) SetName(name string) error {
	self.name = name
	return nil
}

func (self *Source) SetVar(key, val string) (err error) {
	if _, ok := self.vars[key]; !ok {
		self.vars[key] = val
	} else {
		err = fmt.Errorf("variable '%s' already set to '%s'", key, val)
	}
	return
}

func (self *Source) SetHandler(name string) error {

	self.handler = handlers.Search(name, handlers.TYPE_SOURCE)

	return nil
}
