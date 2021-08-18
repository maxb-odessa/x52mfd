package event

import (
	"regexp"

	"elda-go/action"
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

	// get src chan

	// listen src chan

	// process event lines

	// dispatch event to action

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
