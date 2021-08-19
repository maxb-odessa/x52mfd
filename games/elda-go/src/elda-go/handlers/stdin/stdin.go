package stdin

import (
	"fmt"
	"strings"

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
		name: "stdin",
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
	var str string

	fmt.Scanln(&str)

	return strings.TrimSpace(str), nil
}

func (self *handler) Push(s string) error {
	return fmt.Errorf("not implemented")
}
