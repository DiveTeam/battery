import { CURRENT_CHANGE } from '../constants';

export function changeCurrent(current) {
  return {
    type: CURRENT_CHANGE,
    current: current
  }
}
