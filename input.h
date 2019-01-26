enum Input {
	LEFT,
	RIGHT,
	ACTION,
	TOGGLE,
	ARCADE_QUIT,

	// Always at the end
	INPUT_COUNT
};

bool input_array[INPUT_COUNT];
bool once_array[INPUT_COUNT];
