/* movstat/sumacc.c
 * 
 * Copyright (C) 2018 Patrick Alken
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <config.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_errno.h>

typedef double ringbuf_type;
#include "ringbuf.c"

typedef struct
{
  size_t n;       /* window size */
  double sum;     /* current window sum */
  ringbuf *rbuf;  /* ring buffer storing current window */
} sumacc_state_t;

static size_t
sumacc_size(const size_t n)
{
  size_t size = 0;

  size += sizeof(sumacc_state_t);
  size += ringbuf_size(n);

  return size;
}

static int
sumacc_init(const size_t n, void * vstate)
{
  sumacc_state_t * state = (sumacc_state_t *) vstate;

  state->n = n;
  state->sum = 0.0;

  state->rbuf = vstate + sizeof(sumacc_state_t);
  ringbuf_init(n, state->rbuf);

  return GSL_SUCCESS;
}

static int
sumacc_add(const double x, void * vstate)
{
  sumacc_state_t * state = (sumacc_state_t *) vstate;

  if (ringbuf_is_full(state->rbuf))
    {
      /* subtract oldest element from sum */
      state->sum -= ringbuf_peek_back(state->rbuf);
    }

  /* add new element to sum and ring buffer */
  state->sum += x;
  ringbuf_add(x, state->rbuf);

  return GSL_SUCCESS;
}

static int
sumacc_delete(void * vstate)
{
  sumacc_state_t * state = (sumacc_state_t *) vstate;

  if (!ringbuf_is_empty(state->rbuf))
    state->sum -= ringbuf_peek_back(state->rbuf);

  return GSL_SUCCESS;
}

static double
sumacc_get(const void * vstate)
{
  sumacc_state_t * state = (sumacc_state_t *) vstate;
  return state->sum;
}
