package filein

import (
	"bufio"
	"fmt"

	"elda-go/def"
)

// any name
type handler struct {
	// mandatory fields
	name string
	typ  int

	// optional
	scanner *bufio.Scanner
}

// register us
func Register() *handler {
	return &handler{
		name: "filein",
		typ:  def.HANDLER_TYPE_SOURCE,
	}
}

func (self *handler) Init(vars map[string]string) error {

	path, err := def.GetStrEnv(vars, "path")
	if err != nil {
		return err
	}
	if path == "" {
		return fmt.Errorf("variable 'path' must not be empty")
	}

	self.scanner
	return nil
}

func (self *handler) Name() string {
	return self.name
}

func (self *handler) Type() int {
	return self.typ
}

func (self *handler) Pull() (string, error) {
	return "", nil
}

func (self *handler) Push(s string) error {
	return fmt.Errorf("not implemented")
}

func (self *handler) Done() {
}
