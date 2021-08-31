package filein

import (
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"sort"
	"sync"

	"github.com/nxadm/tail"
	"github.com/radovskyb/watcher"

	"elda-go/def"
	"elda-go/log"
)

// any name
type handler struct {
	// mandatory fields
	name string
	typ  int

	// optional
	path    string
	tail    *tail.Tail
	watcher *watcher.Watcher
	lock    sync.Mutex
}

// register us
func Register() *handler {
	return &handler{
		name: "filein",
		typ:  def.HANDLER_TYPE_SOURCE,
	}
}

func (self *handler) Init(vars map[string]string) error {
	var err error
	var path string

	if path, err = def.GetStrVar(vars, "path"); err != nil {
		return err
	}

	// start dir watcher
	if err := self.watchDir(path); err != nil {
		return err
	}

	// start tailer
	go self.tailFile()

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
	// stop watcher
	// stop tailer
}

func (self *handler) watchDir(path string) error {
	var files []string

	walkFunc := func(p string, i os.FileInfo, e error) error {
		if e != nil {
			return e
		}
		ok, err := regexp.MatchString(path, p)
		if ok {
			files = append(files, p)
		}
		return err
	}
	if err := filepath.Walk(filepath.Dir(path)+"/", walkFunc); err != nil {
		return err
	}

	if len(files) > 0 {
		sort.Strings(files)
		self.path = files[len(files)-1]
	}

	log.Debug("filein: tailing file '%v'\n", self.path)

	self.watcher = watcher.New()

	self.watcher.FilterOps(watcher.Create)

	reg := regexp.MustCompile(path)
	self.watcher.AddFilterHook(watcher.RegexFilterHook(reg, false))

	// watch self.dir

	// lock

	return nil
}

func (self *handler) tailFile() {

	// tail(self.dir/self.file)

	// if restart flag - restart tail on new file

}
