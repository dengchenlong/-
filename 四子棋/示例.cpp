#include "ËÄ×ÓÆå.h"

int main()
{
	//get_all_states();
	//save_all_states();
	
	//load_all_states();
	//train((int)1000, print_every_n_epochs, step_size, policy, train_parameter);
	
	//load_all_states();
	//train(print_every_n_epochs, step_size, policy, train_parameter, 7200);
	
	test(1e4, policy, test_parameter);
	
	play(2, policy, test_parameter);
	
	return 0;
}