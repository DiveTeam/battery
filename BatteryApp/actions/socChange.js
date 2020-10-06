import { SOC_CHANGE } from '../constants';

export function changeSOC(soc) {
  return {
    type: SOC_CHANGE,
    soc: soc
  }
}
