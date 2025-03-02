#ifndef __THINGSBOARD_H__
#define __THINGSBOARD_H__

struct ThingsboardData
{
    float temperature;
};

void thingsboard_setup();
void thingsboard_update();
void thingsboard_send_data(struct ThingsboardData const & data);

#endif