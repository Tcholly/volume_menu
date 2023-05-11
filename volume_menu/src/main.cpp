#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <stdexcept>

enum OptionType
{
	OPTION_INCREASE,
	OPTION_DECREASE,
	OPTION_SETTINGS,
	OPTION_MUTE,
	OPTION_UNMUTE
};

struct Option
{
	OptionType type;
	std::string name;
};

struct VolumeState
{
	enum {STATE_MUTE, STATE_UNMUTE} state;
	std::string value;
};


Option options[] =
{
	{OPTION_INCREASE, "+ Increase"},
	{OPTION_DECREASE, "- Decrease"},
	{OPTION_MUTE, "  Unmute"},
	{OPTION_UNMUTE, "  Mute"},
	{OPTION_SETTINGS, "  Settings"},
};

std::string Exec(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

VolumeState GetVolumeState()
{
	VolumeState result; 

	std::string state = Exec("amixer get Master");
	state = state.substr(0, state.find_last_of('\n'));
	state = state.substr(state.find_last_of('\n'));

	auto status_pos = state.find("[on]");
	if (status_pos != state.npos)
		result.state = VolumeState::STATE_UNMUTE;
	else
	{
		result.state = VolumeState::STATE_MUTE;
		status_pos = state.find("[off]");
	}

	std::string volume_str = state.substr(status_pos - 7);
	volume_str = volume_str.substr(volume_str.find('[') + 1);
	volume_str = volume_str.substr(0, volume_str.find("%"));

	result.value = volume_str;
	return result;
}

void HandleOption(const std::string& option)
{
	if (option.empty())
		exit(0);

	for (auto opt : options)
	{
		if (option == opt.name)
		{
			switch (opt.type)
			{
				case OPTION_INCREASE:
					Exec("amixer -Mq set Master,0 5%+ unmute");
					// std::cout << "Incraese volume\n";
					break;

				case OPTION_DECREASE:
					Exec("amixer -Mq set Master,0 5%- unmute");
					// std::cout << "Decrease volume\n";
					break;

				case OPTION_MUTE:
				case OPTION_UNMUTE:
					Exec("amixer set Master toggle");
					// std::cout << "Mute volume\n";
					break;

				case OPTION_SETTINGS:
					Exec("coproc ( pavucontrol > /dev/null  2>&1 )");
					exit(0);
					break;

				default:
					std::cout << "Unreachable: unkown option type: " << (int)opt.type << '\n';
			}
		}
	}

	Exec("set_status.sh --once");
}

void SetRofi()
{
	VolumeState current_state = GetVolumeState();
	for (auto opt : options)
	{
		if (opt.type == OPTION_UNMUTE && current_state.state == VolumeState::STATE_MUTE)
			continue;
		
		if (opt.type == OPTION_MUTE && current_state.state == VolumeState::STATE_UNMUTE)
			continue;

		std::cout << opt.name << '\n';
	}
	
	std::cout << '\0' << "prompt\x1fVolume Menu\n";
	std::cout << '\0' << "message\x1fVolume: " << current_state.value << "%, ";
	switch (current_state.state)
	{
		case VolumeState::STATE_MUTE:
			std::cout << "Mute\n";
			break;
	
		case VolumeState::STATE_UNMUTE:
			std::cout << "Not mute\n";
			break;

		default:
			std::cout << "Unkown state\n";
			break;
	}
}

int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		std::string program = argv[0];
		std::string first_arg = argv[1];
		if (first_arg == "--start")
		{
			std::string command = "rofi -show vm -modes \"vm:" + program + "\" -theme volume_menu/assets/rofi_style.rasi";
			Exec(command);
			return 0;
		}
	}

	for (int i = 1; i < argc; i++)
		HandleOption(argv[i]);

	SetRofi();
}
