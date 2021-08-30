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
	doneCh  chan bool
}

var outChan chan *def.ChanMsg

func init() {
	outChan = make(chan *def.ChanMsg, def.SRC_CHAN_LEN)
}

func GetChan() chan *def.ChanMsg {
	return outChan
}

func New() *Source {
	src := new(Source)
	src.vars = make(map[string]string)
	src.doneCh = make(chan bool)
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

	self.handler = handlers.DupAndZero(handler)

	return nil
}

func (self *Source) Init() error {
	if err := self.handler.Init(self.vars); err != nil {
		return fmt.Errorf("failed to init handler '%s': %v", self.name, err)
	}
	return nil
}

func writeChan(ch chan *def.ChanMsg, msg *def.ChanMsg) (ok bool) {
	defer func() {
		if recover() != nil {
			ok = false
		}
	}()

	ch <- msg

	return true
}

func (self *Source) Run() {
	defer log.Debug("source %s exited\n", self.name)

	go func() {
		for {
			data, err := self.handler.Pull() // must block here

			if err != nil {
				log.Err("source '%s' failed on handler '%s': %v\n", self.name, self.handler.Name(), err)
				return
			}

			if data == "" {
				continue
			}

			msg := &def.ChanMsg{Name: self.name, Data: data}

			log.Debug("source '%s' sending msg '%v'\n", self.name, msg)

			if writeChan(outChan, msg) == false {
				log.Err("source '%s' failed to send data: %v\n", self.name, err)
				return
			}
		}
	}()

	<-self.doneCh

	return
}

func (self *Source) Done() {
	log.Info("stopping source '%s'\n", self.name)
	self.handler.Done()
	self.doneCh <- true
}
