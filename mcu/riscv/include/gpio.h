
#ifndef _GPIO_H_
#define _GPIO_H_

void gpio_set_dir(int group, int bit, int dir);
void gpio_set_data(int group, int bit, int data);

#endif /* _GPIO_H_ */
