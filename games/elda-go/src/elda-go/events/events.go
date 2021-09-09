package events

import (
	"os"
	"os/signal"
	"regexp"
	"strconv"
	"strings"
	"syscall"

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

func Run(sources map[string]*source.Source, actions map[string]*action.Action, events []*Event) {

	doneChan := make(chan bool, 1)
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, os.Interrupt, syscall.SIGTERM)
	go func() {
		for sig := range sigChan {
			log.Info("got signal %d\n", sig)
			doneChan <- true
		}
	}()

	srcChan := source.GetChan()

	for {

		select {
		case msg, ok := <-srcChan:
			if ok {
				process(msg, events)
			}
		case <-doneChan:
			for _, ac := range actions {
				ac.Done()
			}
			for _, sc := range sources {
				sc.Done()
			}
			// close(srcChan)
			return
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
		log.Debug("event matching '%s' to '%v'\n", srcMsg.Data, ev.pattern)
		if ok := ev.pattern.MatchString(srcMsg.Data); !ok {
			log.Debug("event not matched '%s'\n", srcMsg.Data)
			continue
		}

		subs := ev.pattern.FindStringSubmatch(srcMsg.Data)
		log.Debug("event match regex subs: <%v>\n", subs)

		// send message to all actions
		for _, ea := range ev.actions {
			data := ea.data

			// replace regex submatches
			for idx, sub := range subs {
				data = strings.ReplaceAll(data, "$"+strconv.Itoa(idx), sub)
			}

			// replace escape sequences
			data = strings.ReplaceAll(data, `\n`, "\n")
			data = strings.ReplaceAll(data, `\r`, "\r")
			data = strings.ReplaceAll(data, `\t`, "\t")
			data = strings.ReplaceAll(data, `\\`, "\\")

			actMsg := &def.ChanMsg{Name: srcMsg.Name, Data: data}

			select {
			case ea.action.GetChan() <- actMsg:
				log.Debug("event sending '%+v' to action '%s'\n", actMsg, ea.action.Name())
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
