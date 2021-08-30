package stdin

import (
	"bufio"
	"fmt"
	"os"
	"strings"

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
		name: "stdin",
		typ:  def.HANDLER_TYPE_SOURCE,
	}
}

func (self *handler) Init(vars map[string]string) error {
	self.scanner = bufio.NewScanner(os.Stdin)
	return nil
}

func (self *handler) Name() string {
	return self.name
}

func (self *handler) Type() int {
	return self.typ
}

func (self *handler) Pull() (string, error) {
	self.scanner.Scan()
	if err := self.scanner.Err(); err != nil {
		return "", err
	}
	return strings.TrimSpace(self.scanner.Text()), nil
}

func (self *handler) Push(s string) error {
	return fmt.Errorf("not implemented")
}

func (self *handler) Done() {
}
