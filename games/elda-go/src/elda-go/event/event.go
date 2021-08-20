package event

import (
	"regexp"

	"elda-go/action"
	"elda-go/def"
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

func Run(events []*Event) {

	ch := source.GetChan()

	for {
		select {
		case msg := <-ch:
			process(msg, events)
		}
	}

	return
}

func process(srcMsg *def.ChanMsg, events []*Event) {

	// examine all configured events
	for _, ev := range events {

		// chose event from this source only
		if ev.source.Name() != srcMsg.Name {
			continue
		}

		// match event by pattern
		log.Debug("matching %q %+v\n", srcMsg.Data, ev.pattern)
		if ok := ev.pattern.MatchString(srcMsg.Data); !ok {
			log.Debug("not matched %q\n", srcMsg.Data)
			continue
		}

		// send message to all actions
		for _, ea := range ev.actions {

			data := ev.pattern.ReplaceAllString(srcMsg.Data, ea.data)

			actMsg := &def.ChanMsg{Name: srcMsg.Name, Data: data}

			select {
			case ea.action.GetChan() <- actMsg:
				log.Debug("sending '%+v' to action '%s'\n", actMsg, ea.action.Name())
			default:
				log.Warn("action '%s' channel is full\n", ea.action.Name())
			}
		}

	}
}

func New() *Event {
	e := new(Event)
	return e
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
