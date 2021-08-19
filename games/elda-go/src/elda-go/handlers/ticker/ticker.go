package ticker

import (
	"fmt"
	"time"

	"elda-go/def"
)

// any name
type handler struct {
	// mandatory fields
	name string
	typ  int

	// optional
	message string
	ticker  *time.Ticker
}

// register us
func Register() *handler {
	return &handler{
		name: "ticker",
		typ:  def.HANDLER_TYPE_SOURCE,
	}
}

func (self *handler) Init(vars map[string]string) error {

	if i, err := def.GetIntVar(vars, "interval"); err != nil {
		return err
	} else if i <= 0 {
		return fmt.Errorf("variable 'interval' value must be positive")
	} else {
		self.ticker = time.NewTicker(time.Duration(i) * time.Second)
	}

	if m, err := def.GetStrVar(vars, "message"); err != nil {
		return err
	} else {
		self.message = m
	}

	return nil
}

func (self *handler) Name() string {
	return self.name
}

func (self *handler) Type() int {
	return self.typ
}

func (self *handler) Pull() (string, error) {
	select {
	case <-self.ticker.C:
		return self.message, nil
	}
	return "", nil
}

func (self *handler) Push(s string) error {
	return fmt.Errorf("not implemented")
}
