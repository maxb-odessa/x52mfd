package pipein

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
}

// register us
func Register() *handler {
	return &handler{
		name: "pipein",
		typ:  def.HANDLER_TYPE_SOURCE,
	}
}

func (self *handler) Init(vars map[string]string) error {
	return nil
}

func (self *handler) Name() string {
	return self.name
}

func (self *handler) Type() int {
	return self.typ
}

func (self *handler) Pull() (string, error) {
	time.Sleep(time.Second * 1)
	return "", nil
}

func (self *handler) Push(s string) error {
	return fmt.Errorf("not implemented")
}

func (self *handler) Done() {
}
