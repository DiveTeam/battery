import { CURRENT_CHANGE } from '../constants';

const initialState = {
  temperature: 0,
};

const currentReducer = (state = initialState, action) => {
  switch(action.type) {
  case CURRENT_CHANGE:
    return {
      ...state,
      current: action.current
    };
  default:
    return state;
  }
}
export default currentReducer;