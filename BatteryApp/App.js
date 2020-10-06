import React from 'react';
import {
  StyleSheet,
  Button,
  View,
  SafeAreaView,
  Text,
  Alert
} from 'react-native';
import BleComponent from './ble';

import { connect } from 'react-redux';
import { voltage1Change } from './actions/voltage1Change';
import { bindActionCreators } from 'redux';



// const ble = new BleComponent();
// const store = createStore(rootReducer);

class SOC extends React.Component {
  render() {
    return (
      <Text>{"SOC: "}{this.props.soc}{"%"}</Text>
    );
  }
}

class Temperature extends React.Component {
  render() {
    return (
      <Text>{"T: "}{Number(this.props.temperature).toFixed(2)}{"˚C"}</Text>
    );
  }
}

class Current extends React.Component {
  render() {
    return (
      <Text>{"I: "}{Number(this.props.current).toFixed(2)}{"A"}</Text>
    );
  }
}

class Voltage extends React.Component {
  render() {
    return (
      <Text>{"U"}{this.props.v_number}{": "}{Number(this.props.voltage).toFixed(3)}{"V"}</Text>
    );
  }
}

class Wattage extends React.Component {
  render() {
    return (
      <Text>{"P: "}{Number(this.props.wattage).toFixed(1)}{"W"}</Text>
    );
  }
}


class App extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      data: {
        soc: 0,
        current: 0,
        temperature: 0,
        voltage1: 0,
        voltage2: 0,
        voltage3: 0,
        voltage: 0,
        wattage: 0,  
      }
    };
  }

  // updateVoltage(){
  //   this.setState({voltage: (Number(this.state.v1) + Number(this.state.v2) + Number(this.state.v3)).toFixed(3)});
  //   this.updateWattage();
  // }

  // updateWattage(){
  //   this.setState({wattage: (Number(this.state.voltage) * Number(this.state.current)).toFixed(1)});
  // }


  componentDidMount() {
  }

  componentWillUnmount() {
  }

  render() {
    return (
      <SafeAreaView>
        <BleComponent/>
        <View>
          <SOC soc={this.props.soc} />
          <Wattage wattage={this.props.wattage} />
          <Voltage v_number="" voltage={this.props.voltage} />
          <Current current={this.props.current} />
          <Temperature temperature={this.props.temperature} />
          <Voltage v_number="1" voltage={this.props.voltage1} />
          <Voltage v_number="2" voltage={this.props.voltage2} />
          <Voltage v_number="3" voltage={this.props.voltage3} />
        </View>
      </SafeAreaView>
    );
  }
}
// export default App;


const mapStateToProps = state => {
  console.log("STATE:" + JSON.stringify(state));
  const v = (state.voltage1.voltage1 + state.voltage2.voltage2 + state.voltage3.voltage3);
  return {
    voltage1: state.voltage1.voltage1,  //TOOD: get rid of root element
    voltage2: state.voltage2.voltage2,
    voltage3: state.voltage3.voltage3,
    voltage: v,
    current: state.current.current,
    wattage: v * state.current.current,
    temperature: state.temperature.temperature,
    soc: state.soc.soc
   };
};

export default connect(mapStateToProps)(App);