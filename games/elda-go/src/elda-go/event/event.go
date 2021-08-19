package event

import (
	"regexp"

	"elda-go/action"
	"elda-go/log"
	"elda-go/source"
)

type evAction struct {
	action *action.Action
	data   string
}

type Event struct {
	pattern *regexp.Regexp
	source  *source.Source
	actions []*evAction
}

func New() *Event {
	e := new(Event)
	return e
}

func Run(events []*Event) {

	ch := source.GetChan()

	for {
		select {
		case msg := <-ch:
			log.Info("MSG: %+v\n", msg)
			// match msg.data over source.pattern and source.name
			// if match - pass PROCESSED msg to related actions chans
		}
	}

}

func (self *Event) SetSource(src *source.Source, pattern string) (err error) {

	self.pattern, err = regexp.Compile(pattern)
	if err != nil {
		return
	}

	self.source = src

	return nil
}

func (self *Event) AddAction(act *action.Action, data string) (err error) {

	eac := &evAction{
		action: act,
		data:   data,
	}
	self.actions = append(self.actions, eac)

	return nil
}
