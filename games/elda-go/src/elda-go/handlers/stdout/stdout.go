package stdout

import (
	"fmt"

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
		name: "stdout",
		typ:  def.HANDLER_TYPE_ACTION,
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

func (self *handler) Push(s string) error {
	fmt.Println(s)
	return nil
}

func (self *handler) Pull() (string, error) {
	return "", fmt.Errorf("not implemented")
}