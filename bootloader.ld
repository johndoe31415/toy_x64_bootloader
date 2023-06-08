ENTRY(main);
SECTIONS {
	. = 0x7c00;
	.text : {
		_text = .;
		*(.text);
		_text_end = .;
	}
	.data : {
		_data = .;
		*(.bss);
		*(.bss*);
		*(.data);
		*(.rodata*);
		*(COMMON)
		_data_end = .;
	}

	/DISCARD/ : {
		*(.note*);
		*(.iplt*);
		*(.igot*);
		*(.rel*);
		*(.comment);
	}
}
