from tkinter import *
from tkinter import Tk, Label, Button, filedialog
from tkinter.ttk import *
import JoJenWidgets as JJ

#View : User interface elements.
#       --Controller can send messages to it.
#       --View can call methods from Controller vc when an event happens.
#       --NEVER communicates with Model.
#       --Has setters and getters to communicate with controller
class MyView(Frame):
    def __init__(self, parent, controller):
        super().__init__(parent)
        #make the view

        #set the delegate/callback pointer
        self.controller = controller
        self.parent = parent

        #control variables
        self.i_names = StringVar()
        self.s_names = StringVar()
        self.version = IntVar(value=1)
        self.out_dir = StringVar(value='Select Directory...')
        self.out_name = StringVar(value='Default is Instrument Name')
        self.total_sample_size = IntVar(value=0)
        self.inFile = StringVar(value='Select a Soundfont File to Continue')

        #formating variables
        self.box_relief = RIDGE
        self.box_pad = 1
        def_pad = 3

        #set up the widgets/layout
        self.parent.columnconfigure(0, weight=1)
        self.parent.rowconfigure(0, weight=1)
        self.grid(column=0, row=0, sticky=N + S + E + W)
        self.columnconfigure(0, weight=1)
        self.rowconfigure(0, weight=1) # Boxes
        self.rowconfigure(1, weight=6) # ListBoxes
        self.rowconfigure(2, weight=1) # StatusBar

        # main menu
        self.menu = JJ.JJMenu(self.parent)
        self.parent.config(menu=self.menu)
        self.menu.filemenu.add_command(label='Load SF2', command=self.controller.inBrowseSelected)


        self.upper_frame = JJ.JJFrame(self)
        self.upper_frame.rcconfigure(_rows=[(0,1)], _columns=[(0,1),(1,2),(2,3)])
        self.upper_frame.grid(column=0, row=0, columnspan=4, sticky=N + S + E + W, padx=5, pady=5)

        self.box_1 = JJ.JJLabelFrame(self.upper_frame, 2, 1, 1, 1, text='Select Teensy Version')
        self.box_1.grid(column=0, row=0, padx=self.box_pad, pady=self.box_pad, sticky=N + S + E + W)
        self.version_frame = JJ.JJFrame(self.box_1, 2, 1, 1, 1)
        self.version_frame.grid(row=1, padx=def_pad, sticky=N + E + W)
        self.ver_32 = Radiobutton(self.version_frame, text='Teensy 3.2 (default)', variable=self.version, value=1)
        self.ver_32.grid(row=0, column=0, sticky=N + S + E + W, padx=def_pad)
        self.ver_36 = Radiobutton(self.version_frame, text='Teensy 3.6', variable=self.version, value=2)
        self.ver_36.grid(row=1, column=0, sticky=N + S + E + W, padx=def_pad)

        self.box_2 = JJ.JJLabelFrame(self.upper_frame, 2, 1, 1, 1, text='Load a Soundfont')
        self.box_2.grid(column=1, row=0, padx=self.box_pad, pady=self.box_pad, sticky=N + S + E + W)
        # TODO make this text wrap better
        self.infile_label = Message(self.box_2, textvariable=self.inFile, anchor=W, justify=LEFT, aspect=400)
        self.infile_label.grid(row=0, column=0, padx=def_pad, pady=def_pad, sticky=E + W)
        self.browse_button = Button(self.box_2, text="Browse", command=self.controller.inBrowseSelected)
        self.browse_button.grid(row=1, column=0, padx=def_pad, pady=def_pad, sticky=N)

        self.box_3 = JJ.JJLabelFrame(self.upper_frame, 2, 1, 1, 1, text='Output Settings')
        self.box_3.grid(column=2, row=0, padx=self.box_pad, pady=self.box_pad, sticky=N + S + E + W)
        self.dir_line = Frame(self.box_3)
        self.dir_line.grid(row=0, column=0)
        self.folder_label = Label(self.dir_line, text='Folder')
        self.folder_label.grid(row=0, column=0)
        self.folder_entry = Entry(self.dir_line, textvariable=self.out_dir)
        self.folder_entry.grid(row=0, column=1)
        self.folder_button = Button(self.dir_line, text='Browse', command=self.controller.outBrowseSelected)
        self.folder_button.grid(row=0, column=2)
        self.name_label = Label(self.dir_line, text = 'Name')
        self.name_label.grid(row=1, column=0)
        self.name_entry = Entry(self.dir_line, textvariable=self.out_name)
        self.name_entry.grid(row=1, column=1, columnspan=2, sticky=E + W)

        self.lower_frame = JJ.JJFrame(self, 1, 2, 1, 1)
        self.lower_frame.grid(column=0, row=1, columnspan=4, sticky=N + S + E + W, padx=5, pady=5)

        # Instruments
        self.inst_listbox = JJ.JJListBox(self.lower_frame, 'Instruments', self.i_names)
        self.inst_listbox.grid(column=0, row=0, sticky=N + S + E + W, padx=5)

        # Samples
        self.samp_listbox = JJ.JJListBox(self.lower_frame, 'Samples', self.s_names)
        self.samp_listbox.grid(column=1, row=0, sticky=N + S + E + W, padx=5)
        self.samp_listbox.rcconfigure([(0, 1), (1, 16), (2, 1)], [(0, 1)])
        self.samp_listbox.under_frame = JJ.JJFrame(self.samp_listbox, 1, 2, 1, 1)
        self.samp_listbox.under_frame.grid(column=0, row=3)
        self.samp_listbox.decode_button = Button(self.samp_listbox.under_frame, text='Decode', command=self.samplesSelected)
        self.samp_listbox.decode_button.grid(row=0, column=1, padx=5, pady=5, sticky=N + S + E + W)
        # this is for showing total sample size
        self.samp_listbox.samp_size_label = Label(self.samp_listbox.under_frame, textvariable=self.total_sample_size, anchor=W)
        self.samp_listbox.samp_size_label.grid(row=0, column=0, padx=2, pady=2, sticky=N + S + W)

        # Status bar at bottom
        self.status_bar = JJ.JJStatusBar(self, 'Load a Soundfont')
        self.status_bar.grid(row=2, column=0, sticky=E + W + S, padx=0, pady=0)

        # Set Bindings
        self.inst_listbox.list_box.bind('<<ListboxSelect>>', self.instSelected)
        self.samp_listbox.list_box.bind('<<ListboxSelect>>', self.sampleSelected)
        self.samp_listbox.list_box.bind('<Double-1>', self.samplesSelected)

    def instSelected(self, *args):
        self.controller.instrumentSelected(self.inst_listbox.getCurrSelection())
    def samplesSelected(self, *args):
        self.controller.decode(self.samp_listbox.getCurrSelection())
    def sampleSelected(self, *args):
        self.controller.sampleSelected(self.samp_listbox.getCurrSelection())
    #Getters and setters for the control variables.
    def setInstrumentList(self, _newInstruments):
        self.i_names.set(_newInstruments)
    def getInstrumentList(self):
        return self.i_names.get()
    def setSampleList(self, _newSamples):
        self.s_names.set(_newSamples)
    def getSampleList(self):
        return self.s_names.get()
    def setInFile(self, _newFile):
        self.inFile.set(_newFile)
    def getInFile(self):
        return self.inFile.get()
    def setOutDirectory(self, _newDir):
        self.out_dir.set(_newDir)
    def getOutDirectory(self):
        return self.out_dir.get()
    def setOutName(self, _newName):
        self.out_name.set(_newName)
    def getOutName(self):
        self.out_name.get()
    def setStatus(self, _new):
        self.status_bar.setStatus(_new)