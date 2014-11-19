/*
 * The Software License
 * =================================================================================
 * Copyright (c) 2003-2010 The Terimber Corporation. All rights reserved.
 * =================================================================================
 * Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation 
 * and/or other materials provided with the distribution.
 * The end-user documentation included with the redistribution, if any, 
 * must include the following acknowledgment:
 * "This product includes software developed by the Terimber Corporation."
 * =================================================================================
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
 * IN NO EVENT SHALL THE TERIMBER CORPORATION OR ITS CONTRIBUTORS BE LIABLE FOR 
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ================================================================================
*/

#include "threadpool/timer.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

timer::timer() : 
	_ident(0), 
	_interval(INFINITE),
	_multiplier(1),
	_trigger(1),
	_reactivate(false), 
	_first(false), 
	_callback(0)
{
}

timer::~timer()
{
	deactivate();
}

void 
timer::activate(timer_callback* callback, size_t ident, size_t interval, size_t multiplier, bool reactivate)
{
	assert(callback);
	// deactivates timer
	deactivate();
	// sets new parameters
	mutexKeeper keeper(_mtx);
	_callback = callback;
	_ident = ident;
	_interval = interval;
	_reactivate = reactivate;
	// checks zero value of multiplier
	_multiplier = multiplier == 0 ? 1 : multiplier;
	// sets trigger
	_trigger = _multiplier;
	_first = false;

	// prepares task for timer
  	job_task task(this, 0, _interval, 0);
	// starts thread
	_thread.start();
	// assigns job
	_thread.assign_job(task);
}

void 
timer::deactivate()
{
	mutexKeeper keeper(_mtx);
	// cancels job
	_thread.cancel_job();
	// stops thread
	_thread.stop();
}

// virtual 
bool 
timer::v_has_job(size_t ident, void* user_data)
{
	return !_trigger--; // returns the current trigger value and then decrements
}

// virtual 
void 
timer::v_do_job(size_t ident, void* user_data)
{
	// will fire event if first time or will reactivate flag is set 
	if (!_first || _reactivate)
		_callback->notify(_ident, _interval, _multiplier);

	if (!_first) // flips first firing flag
		_first = true;

	// resets trigger
	_trigger = _multiplier;
}

#pragma pack()
END_TERIMBER_NAMESPACE

