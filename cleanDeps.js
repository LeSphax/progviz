const fs = require('fs');

// Read the file
fs.readFile('deps_notion.json', 'utf8', (err, data) => {
  if (err) {
    console.error(err);
    return;
  }

  try {
    // Parse the JSON data
    const fileData = JSON.parse(data);

    // Extract sources and dependencies
    const result = fileData.modules.map((module) => {
      const { source, dependencies } = module;
      return {
        source,
        dependencies: dependencies.map((dependency) => dependency.resolved),
      };
    });

    // Create a new object with sources and dependencies
    const output = { modules: result };

    // Convert the object back to JSON
    const outputJSON = JSON.stringify(output, null, 2);

    // Write the output to a new file
    fs.writeFile('output.json', outputJSON, 'utf8', (err) => {
      if (err) {
        console.error(err);
        return;
      }
      console.log('Output file created successfully.');
    });
  } catch (error) {
    console.error('Error parsing JSON:', error);
  }
});
