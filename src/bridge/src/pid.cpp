/*
 * pid.cpp
 *
 *  Created on: Jun 7, 2017
 *      Author: maximus
 */

#include <math.h>
#include <iostream>

#include "pid.h"

PID::PID(double p, double i, double d, double db, double hi, double lo, double stick, double wu) {
	_kp = p;
	_ki = i;
	_kd = d;
	_dband = db;
	_hi = hi;
	_lo = lo;
	_stiction = stick;
	_windup = wu;

	_out = 0;
	_sum = 0;
	_lasterr = 0;
	_lastsp = 0;

	_lasttime = 0;
}

PID::~PID() {
}

void PID::reconfig(double p, double i, double d, double db, double st, double wu) {
	_kp = p;
	_ki = i;
	_kd = d;
	_dband = db;
	_stiction = st;
	_windup = wu;
}

void PID::reset() {
	_out = _sum = _lasterr = _lastsp = _lasttime = 0;
}

double PID::getNow() {
	struct timespec n;
	clock_gettime(CLOCK_MONOTONIC, &n);
	return (double(n.tv_sec) * 10e9 + n.tv_nsec) / 10e9;
}

double PID::step(double setpoint, double feedback, double now) {

	double err = setpoint - feedback;

	double P = 0;
	double I = 0;
	double D = 0;

	if (now == 0)
		now = getNow();

	if ( _lasttime != 0) {

		P = err * _kp;

		double elapsed = now - _lasttime;

		if (elapsed == 0) {
			return _out;
		}

		_sum += _ki * err * elapsed;
		if (_windup > 0) {
			if (_sum < 0) {
				if (_sum < -_windup)
					_sum = -_windup;
			}
			else {
				if (_sum > _windup)
					_sum = _windup;
			}
		}
		I = _sum;

		D = _kd * ((setpoint - _lastsp) - (err - _lasterr)) / elapsed;
		_lasterr = err;
		_lastsp = setpoint;
	}

	_lasttime = now;

	double delta = P + I + D;

	if (fabs(delta) > _dband)
		_out += delta;

	if (_out > _hi)
		_out = _hi;
	else if (_out < _lo)
		_out = _lo;

	if (fabs(_out) < _stiction)
	  return 0;
	else
	  return _out;
}
