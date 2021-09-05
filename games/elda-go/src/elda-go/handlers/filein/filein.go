package filein

import (
	"fmt"
	"io"
	"path/filepath"
	"regexp"
	"sort"
	"time"

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
	files   []string
	dir     string
	mask    *regexp.Regexp
	tailer  *tail.Tail
	linesCh chan string
	pathCh  chan string
	watcher *watcher.Watcher
}

// register us
func Register() *handler {
	return &handler{
		name: "filein",
		typ:  def.HANDLER_TYPE_SOURCE,
	}
}

func (self *handler) Init(vars map[string]string) error {

	if dir, err := def.GetStrVar(vars, "dir"); err != nil {
		return err
	} else {
		self.dir, _ = filepath.Abs(dir)
		self.dir += "/"
	}

	if mask, err := def.GetStrVar(vars, "mask"); err != nil {
		return err
	} else if self.mask, err = regexp.Compile(mask); err != nil {
		return err
	}

	self.linesCh = make(chan string, 32) // TODO adjust buf size here
	self.pathCh = make(chan string, 1)

	// start dir watcher
	if err := self.watchDir(); err != nil {
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

	select {
	case line, ok := <-self.linesCh:
		if ok {
			return line, nil
		}
	}

	return "", nil
}

func (self *handler) Push(s string) error {
	return fmt.Errorf("not implemented")
}

func (self *handler) Done() {
	// stop dir watcher
	self.watcher.Close()

	// stop tailer
	self.tailer.Stop()
	self.tailer.Cleanup()

	close(self.linesCh)
	close(self.pathCh)
}

func (self *handler) getRecentFile() string {
	if len(self.files) > 0 {
		sort.Strings(self.files)
		return self.files[len(self.files)-1]
	}

	return ""
}

func (self *handler) watchDir() error {

	// monitor the direcotory for newer file to appear

	self.watcher = watcher.New()
	self.watcher.FilterOps(watcher.Create)
	self.watcher.AddFilterHook(watcher.RegexFilterHook(self.mask, false))

	if err := self.watcher.Add(self.dir); err != nil {
		return err
	}

	for path, _ := range self.watcher.WatchedFiles() {
		self.files = append(self.files, path)
	}

	if p := self.getRecentFile(); p != "" {
		self.pathCh <- p
	}

	// start dir watcher
	go func() {
		for {
			select {
			case event := <-self.watcher.Event:
				self.files = append(self.files, event.Path)
				self.pathCh <- self.getRecentFile()
			case err := <-self.watcher.Error:
				log.Err("%v\n", err)
			case <-self.watcher.Closed:
				return
			}
		}
	}()

	go self.watcher.Start(time.Second * 1)

	return nil
}

func (self *handler) tailFile() {
	var err error

	cfg := tail.Config{
		ReOpen: true,
		Follow: true,
		Poll:   true,
		Location: &tail.SeekInfo{
			Offset: 0,
			Whence: io.SeekEnd},
	}
	self.tailer, _ = tail.TailFile("/dev/null", cfg)

	for {
		select {
		case path, ok := <-self.pathCh:
			if !ok {
				break
			}
			log.Debug("tailing file '%s'\n", path)
			self.tailer.Stop()
			self.tailer.Cleanup()
			self.tailer, err = tail.TailFile(path, cfg)
			if err != nil {
				log.Err("tailer: %v\n", err)
			}
		case line, ok := <-self.tailer.Lines:
			if !ok {
				continue
			}
			self.linesCh <- line.Text
		}
	}

}
