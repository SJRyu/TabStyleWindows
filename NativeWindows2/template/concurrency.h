#pragma once

#include <ppl.h>
#include <ppltasks.h>
#include <concrt.h>

namespace Concurrency
{
	void inline Delay(unsigned int ms, cancellation_token& ct)
	{
		event e;
		cancellation_token_registration cb;
		cb = ct.register_callback(
			[&e, ct, &cb]()
			{
				ct.deregister_callback(cb);
				e.set();
			}
		);
		e.wait(ms);
	}
}
