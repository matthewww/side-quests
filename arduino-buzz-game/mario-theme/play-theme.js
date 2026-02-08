async function playTheme() {
  const response = await fetch('./mario-theme.json');
  const theme = await response.json();

  const audioContext = new (window.AudioContext || window.webkitAudioContext)();

  function playNote(frequency, duration) {
    return new Promise((resolve) => {
      const oscillator = audioContext.createOscillator();
      const gainNode = audioContext.createGain();

      oscillator.connect(gainNode);
      gainNode.connect(audioContext.destination);

      oscillator.type = 'square';
      oscillator.frequency.value = frequency;

      gainNode.gain.setValueAtTime(0, audioContext.currentTime);
      gainNode.gain.linearRampToValueAtTime(1, audioContext.currentTime + 0.01);

      oscillator.start(audioContext.currentTime);

      setTimeout(() => {
        gainNode.gain.linearRampToValueAtTime(0, audioContext.currentTime + 0.01);
        oscillator.stop(audioContext.currentTime + 0.01);
        resolve();
      }, duration);
    });
  }

  for (let i = 0; i < theme.frequencies.length; i++) {
    const frequency = theme.frequencies[i];
    const duration = theme.durations[i];

    if (frequency > 0) {
      await playNote(frequency, duration);
    }

    await new Promise(resolve => setTimeout(resolve, duration * 0.30));
  }
}

// Create a button to start the theme
const button = document.createElement('button');
button.textContent = 'Play Mario Theme';
document.body.appendChild(button);

button.addEventListener('click', playTheme);
