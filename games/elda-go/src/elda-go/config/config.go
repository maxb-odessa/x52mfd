package config

import (
	"bufio"
	"fmt"
	"os"
	"strings"
	"unicode"

	"elda-go/action"
	"elda-go/events"
	"elda-go/source"
)

type Cfg struct {
	confLines []string
	confLen   int
	lineNo    int
	sources   map[string]*source.Source
	actions   map[string]*action.Action
	events    []*events.Event
}

func New(confFile string) (*Cfg, error) {
	var cfg Cfg

	// read config file into a map
	fp, err := os.Open(confFile)
	if err != nil {
		return nil, err
	}

	scanner := bufio.NewScanner(fp)
	scanner.Split(bufio.ScanLines)

	for scanner.Scan() {
		cfg.confLines = append(cfg.confLines, scanner.Text())
	}

	fp.Close()

	cfg.confLen = len(cfg.confLines)
	cfg.sources = make(map[string]*source.Source)
	cfg.actions = make(map[string]*action.Action)

	return &cfg, nil
}

func (self *Cfg) Sources() map[string]*source.Source {
	return self.sources
}

func (self *Cfg) Actions() map[string]*action.Action {
	return self.actions
}

func (self *Cfg) Events() []*events.Event {
	return self.events
}

func (self *Cfg) Parse() (err error) {

	for self.lineNo = 0; self.lineNo < self.confLen; self.lineNo++ {

		// remove all leading and trailing blanks
		l := strings.TrimSpace(self.confLines[self.lineNo])

		// skip comments and empty lines
		if len(l) == 0 || l[0] == '#' {
			continue
		}

		// this is a block start
		switch l {
		case "source":
			err = self.parseSource()
		case "action":
			err = self.parseAction()
		case "event":
			err = self.parseEvent()
		default:
			err = fmt.Errorf("source/action/event expected")
		}

		if err != nil {
			err = fmt.Errorf("error at line %d near \"%16s\": %v", self.lineNo+1, self.confLines[self.lineNo], err)
			return
		}
	}

	return
}

func (self *Cfg) parseSource() (err error) {

	src := source.New()

	defer func() {
		if err == nil {
			self.sources[src.Name()] = src
		}
	}()

	// will start with next line
	for self.lineNo++; self.lineNo < self.confLen; self.lineNo++ {
		line := self.confLines[self.lineNo]

		// skip empty lines and comments
		if line2 := strings.TrimSpace(line); len(line2) == 0 || line2[0] == '#' {
			continue
		}

		// this block is done: return and rewind lines pointer by one
		// so the caller will see the line before we stopped processing
		if unicode.IsSpace(rune(line[0])) == false {
			self.lineNo = self.lineNo - 1
			return
		}

		line = strings.TrimSpace(line)

		// we expect "keyword key value..."
		tokens := fieldsN(line, 3)
		switch len(tokens) {
		case 1:
			err = fmt.Errorf("incomplete line")
			return
		case 2:
			tokens = append(tokens, "")
		}

		// tokens #0 and #1 are defined, token #2 is optional
		switch tokens[0] {
		case "name":
			if _, ok := self.sources[tokens[1]]; !ok {
				err = src.SetName(tokens[1])
			} else {
				err = fmt.Errorf("source name '%s' already defined", tokens[1])
			}
		case "var":
			err = src.SetVar(tokens[1], tokens[2])
		case "handler":
			err = src.SetHandler(tokens[1])
		default:
			err = fmt.Errorf("unknown token for source")
		}

		if err != nil {
			return
		}

	}

	return nil
}

// TODO this func is very similar to parseSource()
func (self *Cfg) parseAction() (err error) {

	act := action.New()

	defer func() {
		if err == nil {
			self.actions[act.Name()] = act
		}
	}()

	// will start with next line
	for self.lineNo++; self.lineNo < self.confLen; self.lineNo++ {
		line := self.confLines[self.lineNo]

		// skip empty lines and comments
		if line2 := strings.TrimSpace(line); len(line2) == 0 || line2[0] == '#' {
			continue
		}

		// this block is done: return and rewind lines pointer by one
		// so the caller will see the line before we stopped processing
		if unicode.IsSpace(rune(line[0])) == false {
			self.lineNo = self.lineNo - 1
			return
		}

		line = strings.TrimSpace(line)

		// we expect "keyword key value..."
		tokens := fieldsN(line, 3)
		switch len(tokens) {
		case 1:
			err = fmt.Errorf("incomplete line")
			return
		case 2:
			tokens = append(tokens, "")
		}

		// tokens #0 and #1 are defined, token #2 is optional
		switch tokens[0] {
		case "name":
			if _, ok := self.actions[tokens[1]]; !ok {
				err = act.SetName(tokens[1])
			} else {
				err = fmt.Errorf("action name '%s' already defined", tokens[1])
			}
		case "var":
			err = act.SetVar(tokens[1], tokens[2])
		case "handler":
			err = act.SetHandler(tokens[1])
		default:
			err = fmt.Errorf("unknown token for action")
		}

		if err != nil {
			return
		}

	}

	return nil
}

func (self *Cfg) parseEvent() (err error) {

	ev := events.New()

	defer func() {
		if err == nil {
			self.events = append(self.events, ev)
		}
	}()

	// will start with next line
	for self.lineNo++; self.lineNo < self.confLen; self.lineNo++ {
		line := self.confLines[self.lineNo]

		// skip comments and empty lines
		if line2 := strings.TrimSpace(line); len(line2) == 0 || line2[0] == '#' {
			continue
		}

		// this block is done: return and rewind lines pointer by one
		// so the caller will see the line before we stopped processing
		if unicode.IsSpace(rune(line[0])) == false {
			self.lineNo = self.lineNo - 1
			return
		}

		line = strings.TrimSpace(line)

		// we expect "keyword key value..."
		tokens := fieldsN(line, 3)
		if len(tokens) < 3 {
			err = fmt.Errorf("incomplete line")
			return
		}

		// tokens #0 and #1 are defined, token #2 is optional
		switch tokens[0] {
		case "source":
			if s, ok := self.sources[tokens[1]]; ok {
				err = ev.SetSource(s, tokens[2])
			} else {
				err = fmt.Errorf("undefine source '%s'", tokens[1])
			}
		case "action":
			if a, ok := self.actions[tokens[1]]; ok {
				err = ev.AddAction(a, tokens[2])
			} else {
				err = fmt.Errorf("undefine action '%s'", tokens[1])
			}
		default:
			err = fmt.Errorf("unknown token for event")
		}

		if err != nil {
			return
		}

	}

	return nil
}

func fieldsN(str string, n int) []string {
	count := 0
	prevSep := false

	return strings.FieldsFunc(str, func(r rune) bool {
		if count >= n-1 {
			return false
		}
		if unicode.IsSpace(r) {
			if prevSep == false {
				count++
				prevSep = true
			}
			return true
		}
		prevSep = false
		return false
	})
}
