package source

import (
	"fmt"

	"elda-go/def"
	"elda-go/handlers"
	"elda-go/log"
)

type Source struct {
	name    string
	vars    map[string]string
	handler handlers.Handler
}

type ChanMsg struct {
	name string
	data string
}

var outChan chan *ChanMsg

func init() {
	outChan = make(chan *ChanMsg, def.SRC_CHAN_LEN)
}

func GetChan() chan *ChanMsg {
	return outChan
}

func New() *Source {
	src := new(Source)
	src.vars = make(map[string]string)
	return src
}

func (self *Source) Name() string {
	return self.name
}

func (self *Source) SetName(name string) error {
	if self.name != "" {
		return fmt.Errorf("can not set source name to '%s': already set to '%s'", name, self.name)
	}
	self.name = name
	return nil
}

func (self *Source) SetVar(key, val string) error {
	if v, ok := self.vars[key]; ok {
		return fmt.Errorf("variable '%s' already set to '%s'", key, v)
	}
	self.vars[key] = val
	return nil
}

func (self *Source) SetHandler(name string) error {

	if self.handler != nil {
		return fmt.Errorf("handler already defined")
	}

	handler := handlers.Search(name, def.HANDLER_TYPE_SOURCE)
	if handler == nil {
		return fmt.Errorf("handler '%s' does not exist", name)
	}

	self.handler = handlers.New(handler)

	if err := handler.Init(self.vars); err != nil {
		return fmt.Errorf("failed to init handler '%s': %v", name, err)
	}

	// init handler
	return nil
}

func (self *Source) SendMsg(data string) error {
	outChan <- &ChanMsg{name: self.name, data: data}
	return nil
}

func (self *Source) Run() {
	for {
		if data, err := self.handler.Pull(); err != nil {
			log.Err("source '%s' error in handler '%s': %v\n", self.Name(), self.handler.Name(), err)
		} else {
			self.SendMsg(data)
		}
	}
}
