package action

import (
	"fmt"

	"elda-go/def"
	"elda-go/handlers"
	"elda-go/log"
)

type Action struct {
	name    string
	vars    map[string]string
	handler handlers.Handler
	inChan  chan *def.ChanMsg
}

func New() *Action {
	act := new(Action)
	act.vars = make(map[string]string)
	act.inChan = make(chan *def.ChanMsg, def.ACT_CHAN_LEN)
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

	handler := handlers.Search(name, def.HANDLER_TYPE_ACTION)
	if handler == nil {
		return fmt.Errorf("handler '%s' does not exist", name)
	}

	self.handler = handlers.DupAndZero(handler)

	return nil
}

func (self *Action) GetMsg() (string, string, error) {
	select {
	case msg := <-self.inChan:
		return msg.Name, msg.Data, nil
	}
	return "", "", fmt.Errorf("erhm... what?!")
}

func (self *Action) Init() error {
	if err := self.handler.Init(self.vars); err != nil {
		return fmt.Errorf("failed to init handler '%s': %v", self.name, err)
	}
	return nil
}

func (self *Action) Run() {
	for {

		src, str, err := self.GetMsg()
		if str == "" {
			continue
		}

		if err != nil {
			log.Warn("action '%s' failed: %v\n", self.name, err)
			continue
		}

		log.Info("action '%s' got msg from '%s': [%s]\n", src, str)

		if err = self.handler.Push(str); err != nil {
			log.Warn("action '%s' error in handler '%s' failed: %v\n", self.name, self.handler.Name(), err)
		}
	}

	return
}
